#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory> // For std::unique_ptr

// LLVM temel başlıkları
#include "llvm/ADT/Optional.h" // Modern LLVM'de Optional kullanımı
#include "llvm/IR/IRBuilder.h" // LLVM IR oluşturmak için
#include "llvm/IR/LLVMContext.h" // LLVM bağlamı
#include "llvm/IR/Module.h"     // LLVM Modülü (tüm kodun ana kabı)
#include "llvm/IR/Verifier.h"   // LLVM IR'yi doğrulamak için
#include "llvm/Passes/PassBuilder.h" // Modern optimizasyon geçişleri için
#include "llvm/Passes/OptimizationLevel.h" // Optimizasyon seviyeleri için enum

// LLVM Hedef (Target) başlıkları
#include "llvm/Support/TargetSelect.h" // Hedefleri başlatmak için
#include "llvm/Support/TargetRegistry.h" // Hedef bulmak için
#include "llvm/Target/TargetMachine.h" // Hedef makine (mimariye özgü ayarlar)
#include "llvm/MC/TargetRegistry.h" // Modern TargetRegistry için

// LLVM Dosya/Girdi/Çıktı başlıkları
#include "llvm/Support/FileSystem.h" // Dosya işlemleri için
#include "llvm/Support/Host.h"       // Host bilgilerini almak için
#include "llvm/Support/raw_ostream.h" // LLVM'in çıktı akışları için

// Komut satırı argümanları başlığı (LLVM'in kendi kütüphanesini kullanabiliriz)
#include "llvm/Support/CommandLine.h"

// Linker başlığı (Genellikle lld harici bir araç olarak çağrılır, doğrudan C++ API nadiren kullanılır)
 #include "llvm/Linker/Linker.h" // Doğrudan lld C++ API kullanımı karmaşıktır

// Sistem hata başlığı
#include <system_error>

// Kendi derleyici bileşenlerinizin başlıkları (Placeholder - Sizin tarafınızdan oluşturulacaklar)
#include "lexer.h"
#include "parser.h"
#include "sema.h" // Semantic Analyzer (Anlamsal Analizci)
#include "codegen.h" // LLVM IR Code Generator (Kod Üretici)
#include "module_manager.h" // Modül ve Import yönetimi
#include "diagnostics.h" // Hata ve uyarı raporlama sistemi
 #include "ast.h" // AST düğüm tanımları (eğer ayrı dosyalardaysa)


// ==============================================================================
// Komut Satırı Argümanlarının Tanımlanması (LLVM'in CommandLine kütüphanesi ile)
// Daha karmaşık argüman yönetimi için başka kütüphaneler (plusa, boost::program_options) kullanılabilir.
// ==============================================================================

// Giriş dosyası (.cnt)
llvm::cl::opt<std::string> InputFilename(llvm::cl::Positional,
                                          llvm::cl::desc("Input .cnt file"),
                                          llvm::cl::Required,
                                          llvm::cl::value_desc("filename"));

// Çıktı dosyası
llvm::cl::opt<std::string> OutputFilename("o",
                                           llvm::cl::desc("Output file name"),
                                           llvm::cl::value_desc("filename"),
                                           llvm::cl::init("a.out")); // Varsayılan çıktı adı

// Optimizasyon seviyesi
llvm::cl::opt<std::string> OptLevel("O",
                                    llvm::cl::desc("Optimization level (0, 2, s, x, 3, fast)"),
                                    llvm::cl::value_desc("level"),
                                    llvm::cl::init("0")); // Varsayılan -O0

// Hedef mimari (İlk aşamada RISC-V'ye sabit, ancak komut satırı seçeneği olarak eklenebilir)
 llvm::cl::opt<std::string> TargetTriple("target",
                                          llvm::cl::desc("Target triple (e.g., riscv64-unknown-elf)"),
                                          llvm::cl::value_desc("triple"),
                                          llvm::cl::init(llvm::sys::getDefaultTargetTriple())); // Varsayılan sistem mimarisi

// Çeşitli çıktı formatları (İsteğe bağlı olarak IR, Assembly çıktısı vb.)
 llvm::cl::opt<bool> EmitLLVMIR("emit-llvm", llvm::cl::desc("Emit LLVM IR"), llvm::cl::init(false));
 llvm::cl::opt<bool> EmitAssembly("emit-asm", llvm::cl::desc("Emit Assembly"), llvm::cl::init(false));


// ==============================================================================
// Ana Derleyici Fonksiyonu
// ==============================================================================

int main(int argc, char** argv) {
    // 1. LLVM Altyapısını Başlatma
    // Gerekli tüm LLVM bileşenlerini başlatın. Bu genellikle derleyicinin başında bir kez yapılır.
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    // 2. Komut Satırı Argümanlarını Ayrıştırma
    llvm::cl::ParseCommandLineOptions(argc, argv, "CNT Compiler: A new programming language compiler built with LLVM\n");

    // 3. Optimizasyon Seviyesini Belirleme
    llvm::OptimizationLevel optimizationLevel;
    if (OptLevel == "0") {
        optimizationLevel = llvm::OptimizationLevel::O0;
    } else if (OptLevel == "2") {
        optimizationLevel = llvm::OptimizationLevel::O2;
    } else if (OptLevel == "s") {
        optimizationLevel = llvm::OptimizationLevel::Os;
    } else if (OptLevel == "x") { // -Ox ve -O3 için agresif optimizasyon seviyesi
        optimizationLevel = llvm::OptimizationLevel::O3;
    } else if (OptLevel == "3") {
        optimizationLevel = llvm::OptimizationLevel::O3;
    } else if (OptLevel == "fast") { // -Ofast için agresif optimizasyon seviyesi (floating point semantics değişebilir)
        optimizationLevel = llvm::OptimizationLevel::Ofast;
    } else {
        // diagnostics.h dosyasındaki ReportError fonksiyonunu kullanarak hata bildirin
        ReportError("Hata: Desteklenmeyen optimizasyon seviyesi '" + OptLevel + "'. Desteklenenler: 0, 2, s, x, 3, fast.");
        return 1; // Hata kodu ile çıkış yap
    }

    // 4. Hedef Mimariyi Yapılandırma (İlk aşamada RISC-V)
    // Gerçek bir derleyici hedef üçlüsünü (target triple) genellikle komut satırından alır veya sistemden belirler.
    // RISC-V için yaygın üçlüler: "riscv64-unknown-elf", "riscv32-unknown-elf"
    std::string targetTriple = "riscv64-unknown-elf"; // Veya komut satırından gelen değeri kullanın

    std::string Error;
    // TargetRegistry'den hedefi bulun
    auto Target = llvm::TargetRegistry::lookupTarget(targetTriple, Error);
    if (!Target) {
        ReportError("Hata: Hedef mimari bulunamadı: " + Error);
        return 1;
    }

    // Hedef makineyi oluşturun (mimariye özgü detayları içerir)
    llvm::TargetOptions opt;
    // opt nesnesini TargetMachineOptions (llvm/Target/TargetOptions.h) ile yapılandırabilirsiniz
    // opt.FloatABIType = llvm::FloatABI::Hard; // Örnek: Hard-float ABI kullan

    auto TargetMachine = Target->createTargetMachine(
        targetTriple,          // Hedef üçlüsü
        "generic",             // CPU türü (belirli bir CPU varyantı "rocket", "sifive-e76" vb. olabilir)
        "",                    // CPU özellikleri (özelleştirmeler "+m,+a,+f,+d" gibi)
        opt,                   // Hedef seçenekleri
        llvm::Reloc::PIC_,     // Relocation modeli (Position Independent Code)
        llvm::CodeModel::Default, // Kod modeli
        optimizationLevel      // Optimizasyon seviyesini burada TargetMachine'e geçirin
    );

    if (!TargetMachine) {
        ReportError("Hata: Hedef makine oluşturulamadı.");
        return 1;
    }

    // 5. Giriş Kaynak Kodunu Oku
    std::ifstream sourceFile(InputFilename);
    if (!sourceFile.is_open()) {
        ReportError("Hata: Giriş dosyası açılamadı: " + InputFilename);
        return 1;
    }
    std::string sourceCode((std::istreambuf_iterator<char>(sourceFile)),
                           (std::istreambuf_iterator<char>()));
    sourceFile.close();

    // 6. Derleyici Ön Uç Aşamaları (Frontend): Lexing, Parsing, Semantic Analysis, LLVM IR Generation
    // Bu kısım sizin dilinizin yapısına özel kodunuzu içerecektir.
    // Placeholders:

    Lexer lexer(sourceCode);
    Parser parser(lexer);
    auto ast = parser.parse(); // Kaynak kodu ayrıştır ve AST'yi oluştur

    if (!ast) {
        // Ayrıştırma hatası oluştuysa, hata zaten parser veya diagnostics tarafından bildirilmiş olmalı
        ReportError("Hata: Sözdizimsel ayrıştırma başarısız."); // Daha spesifik hata parser'dan gelmeli
        return 1;
    }

    SemanticAnalyzer sema;
    // Anlamsal analiz yap: Tip kontrolü, kapsam, sahiplik/ödünç alma kuralları, arayüz çözümlemesi
    bool semanticErrors = sema.analyze(ast);

    if (semanticErrors) {
        ReportError("Hata: Anlamsal analiz sırasında hatalar bulundu.");
        return 1;
    }

    // LLVM Bağlamı ve Modülü Oluşturma
    // LLVMContext tüm LLVM veri yapıları için ana konteynerdir. Çoğu uygulama için bir tane yeterlidir.
    llvm::LLVMContext TheContext;
    // LLVM Modülü, tüm fonksiyonları ve global değişkenleri içerir. Bir kaynak dosyaya karşılık gelir.
    auto TheModule = std::make_unique<llvm::Module>(InputFilename, TheContext);

    // Modülün hedef üçlüsünü ve veri düzenini ayarla
    TheModule->setTargetTriple(TargetMachine->getTargetTriple().str());
    TheModule->setDataLayout(TargetMachine->createDataLayout());

    // Kod Üreticiyi (Code Generator) başlat ve LLVM IR üret
    LLVMCodeGenerator codegen(*TheModule, TheContext, *TargetMachine); // CodeGen'in ihtiyaç duyduğu argümanları geçirin
    bool codegenErrors = codegen.generate(ast); // AST'den LLVM IR'yi üret

    if (codegenErrors) {
        ReportError("Hata: Kod üretimi sırasında hatalar bulundu.");
        return 1;
    }

    // İsteğe bağlı: Üretilen LLVM IR'yi doğrula (hataları yakalamak için iyi bir adım)
    if (llvm::verifyModule(*TheModule, &llvm::errs())) {
        ReportError("Hata: Üretilen LLVM IR geçersiz. Derleme durduruldu.");
        // Geçersiz modülü görmek isterseniz: TheModule->print(llvm::errs(), nullptr);
        return 1;
    }

    // İsteğe bağlı: Üretilen LLVM IR'yi bir dosyaya yaz (Hata ayıklama için)
     if (EmitLLVMIR) {
         std::error_code EC;
         llvm::raw_file_ostream dest("output.ll", EC, llvm::sys::fs::OF_None);
         if (EC) {
             ReportError("Hata: LLVM IR dosyası yazılamadı: " + EC.message());
             return 1;
         }
         TheModule->print(dest, nullptr);
     }


    // 7. LLVM Optimizasyon Geçişlerini Çalıştırma
    // Modern LLVM'de PassBuilder kullanılır.
    llvm::LoopAnalysisManager LAM;
    llvm::FunctionAnalysisManager FAM;
    llvm::CGSCCAnalysisManager CGAM;
    llvm::ModuleAnalysisManager MAM;

    llvm::PassBuilder PB(TargetMachine); // PassBuilder hedef makineyi bilir ve buna göre geçişleri ayarlar

    // Gerekli analizleri kaydet
    PB.registerModuleAnalyses(MAM);
    PB.registerCGSCCAnalyses(CGAM);
    PB.registerFunctionAnalyses(FAM);
    PB.registerLoopAnalyses(LAM);
    PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

    // Optimizasyon geçişlerini PassManager'a ekle
    llvm::ModulePassManager MPM = PB.buildPerModuleDefaultPipeline(optimizationLevel);

    // Optimizasyonları çalıştır
    MPM.run(*TheModule, MAM);


    // 8. Makine Kodunu Üretme (Object Dosyası veya Assembly)
    // Çıktı dosyası uzantısına göre Assembly mi yoksa Object file mı üretileceğine karar verilebilir.
    llvm::CodeGenFileType FileType = llvm::CodeGenFileType::ObjectFile; // Varsayılan olarak Object File üret

    if (OutputFilename.empty()) {
         // Çıktı adı belirtilmediyse, giriş dosyasından '.cnt' uzantısını kaldırıp '.o' ekle
        size_t lastDot = InputFilename.rfind('.');
        if (lastDot == std::string::npos) {
            OutputFilename = InputFilename + ".o";
        } else {
            OutputFilename = InputFilename.substr(0, lastDot) + ".o";
        }
    }

    // İsteğe bağlı olarak, çıktı uzantısına göre FileType'ı belirle
     if (OutputFilename.size() > 2 && OutputFilename.substr(OutputFilename.size() - 2) == ".s") {
         FileType = llvm::CodeGenFileType::AssemblyFile;
     }


    std::error_code EC;
    // Çıktı dosyasını aç (ikili çıktı için OF_None)
    llvm::raw_fd_ostream dest(OutputFilename, EC, llvm::sys::fs::OF_None);
    if (EC) {
        ReportError("Hata: Çıktı dosyası '" + OutputFilename + "' açılamadı: " + EC.message());
        return 1;
    }

    // Hedef makinenin pass manager'ına kod üretimi için gerekli geçişleri ekle
    llvm::legacy::PassManager codeGenPM; // Eski PassManager hala kod üretimi için yaygın
    if (TargetMachine->addPassesToEmitFile(codeGenPM, dest, nullptr, FileType)) {
        ReportError("Hata: Hedef makine istenen dosya türünü üretemiyor.");
        return 1;
    }

    // Kod üretimi geçişlerini çalıştır
    codeGenPM.run(*TheModule);
    dest.flush(); // Tamponu boşalt ve dosyaya yaz


    // 9. Linkleme (Genellikle harici lld aracılığıyla)
    // LLVM'in C++ API'si ile doğrudan linkleme karmaşık olabilir.
    // Genellikle bu aşamada, derleyici sürücüsü (sizin main.cpp'niz),
    // üretilen object (.o) dosyalarını ve gerekli kütüphaneleri alarak
    // lld (LLVM Linker) gibi harici bir linker programını çalıştırır.
    // Bu örnek, sadece object dosyası üretme aşamasında durmaktadır.
    // Çalıştırılabilir dosya elde etmek için lld'yi çağırmanız gerekir.

    // Örnek (basit ve güvenli olmayan) lld çağırma konsepti:
     std::string executableFilename = "a.out"; // Veya ayrı bir komut satırı argümanından al
     std::string linkerCommand = "lld " + OutputFilename + " -o " + executableFilename;
     int linkerResult = system(linkerCommand.c_str()); // system() kullanmak genellikle kaçınılmalıdır
     if (linkerResult != 0) {
         ReportError("Hata: Linkleme başarısız.");
         return 1;
     }


    // 10. Başarılı Tamamlama Mesajı
    ReportInfo("Derleme başarıyla tamamlandı: " + InputFilename + " -> " + OutputFilename);

    return 0; // Başarılı çıkış kodu
}

// Diagnostics fonksiyonlarının temel implementasyonu (diagnostics.h'ta tanımlanmalı)
// extern keyword'ünü kaldırın veya diagnostics.h'ta tanımlayıp burada implemente edin.
 void ReportError(const std::string& message) {
     std::cerr << "HATA: " << message << std::endl;
 }
//
 void ReportInfo(const std::string& message) {
     std::cout << "Bilgi: " << message << std::endl;
 }
