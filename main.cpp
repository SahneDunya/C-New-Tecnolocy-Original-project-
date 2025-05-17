#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <filesystem> // C++17 için dosya sistemi işlemleri

// LLVM temel başlıkları
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h" // Veya llvm/Passes/PassBuilder.h

// Kendi derleyici bileşenlerinizin başlıkları
#include "diagnostics.h"           // Hata raporlama
#include "token.h"                 // Token ve TokenLocation
#include "lexer.h"                 // Lexer
#include "parser.h"                // Parser
#include "ast.h"                   // AST temel
#include "expressions.h"           // AST ifadeler
#include "statements.h"            // AST deyimler
#include "declarations.h"          // AST bildirimler
#include "types.h"                 // AST tipler
#include "symbol_table.h"          // Sembol Tablosu
#include "type_system.h"           // Tip Sistemi
#include "ownership_checker.h"     // Sahiplik/Ödünç Alma Kontrolcüsü
#include "module_interface.h"      // Modül Arayüz Yapısı
#include "module_manager.h"        // Modül Çözümleyici
#include "interface_extractor.h"   // Arayüz Çıkarıcı
#include "utils.h"                 // Genel yardımcılar (örn: cnt_compiler::splitString)


// ==============================================================================
// Komut Satırı Argümanlarının Tanımlanması (LLVM'in CommandLine kütüphanesi ile)
// ==============================================================================

// Giriş dosyası (.cnt)
llvm::cl::opt<std::string> InputFilename(llvm::cl::Positional,
                                          llvm::cl::desc("Input .cnt file"),
                                          llvm::cl::Required,
                                          llvm::cl::value_desc("filename"));

// Çıktı Object (.o) dosyası adı
llvm::cl::opt<std::string> OutputObjectFilename("o",
                                               llvm::cl::desc("Output object file name"),
                                               llvm::cl::value_desc("filename"));

// Çıktı Interface (.hnt) dosyası adı veya dizini
// Kullanıcı tam adı belirtebilir (-hnt mylib.hnt) veya sadece dizini belirtebilir (-hntdir include/)
llvm::cl::opt<std::string> OutputInterfaceLocation("hnt",
                                                   llvm::cl::desc("Output interface (.hnt) file name or directory"),
                                                   llvm::cl::value_desc("path"));

// Optimizasyon seviyesi
llvm::cl::opt<std::string> OptLevel("O",
                                    llvm::cl::desc("Optimization level (0, 2, s, x, 3, fast)"),
                                    llvm::cl::value_desc("level"),
                                    llvm::cl::init("0")); // Varsayılan -O0

// İmport arama yolları
llvm::cl::list<std::string> ImportSearchPaths("I",
                                             llvm::cl::desc("Add directory to import search path"),
                                             llvm::cl::value_desc("directory"));

// Hedef mimari (Sabit veya Komut Satırı Seçeneği)
 llvm::cl::opt<std::string> TargetTriple("target", ...);


// ==============================================================================
// Ana Derleyici Fonksiyonu
// ==============================================================================

int main(int argc, char** argv) {
    // 1. LLVM Altyapısını Başlatma
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    // 2. Komut Satırı Argümanlarını Ayrıştırma
    llvm::cl::ParseCommandLineOptions(argc, argv, "CNT Compiler\n");

    // 3. Gerekli Derleyici Bileşenlerini Oluşturma
    Diagnostics diagnostics;
    TypeSystem typeSystem;
    SymbolTable symbolTable; // Ana sembol tablosu
    OwnershipChecker ownershipChecker(diagnostics, typeSystem); // OwnershipChecker'ın ihtiyaç duyduğu bağımlılıkları verin
    ModuleResolver moduleResolver(diagnostics, typeSystem /*, symbolTable*/); // ModuleResolver'ın ihtiyaç duyduğu bağımlılıkları verin
    InterfaceExtractor interfaceExtractor(diagnostics, typeSystem); // InterfaceExtractor'ın ihtiyaç duyduğu bağımlılıkları verin


    // 4. Optimizasyon Seviyesini Belirleme
    llvm::OptimizationLevel optimizationLevel;
    if (OptLevel == "0") optimizationLevel = llvm::OptimizationLevel::O0;
    else if (OptLevel == "2") optimizationLevel = llvm::OptimizationLevel::O2;
    else if (OptLevel == "s") optimizationLevel = llvm::OptimizationLevel::Os;
    else if (OptLevel == "x" || OptLevel == "3") optimizationLevel = llvm::OptimizationLevel::O3; // Ox ve O3'ü O3'e haritalayalım
    else if (OptLevel == "fast") optimizationLevel = llvm::OptimizationLevel::Ofast; // Ofast
    else {
        diagnostics.reportError("Geçersiz optimizasyon seviyesi: " + OptLevel + ". Desteklenenler: 0, 2, s, x, 3, fast.");
        diagnostics.printAll();
        return diagnostics.hasErrors() ? 1 : 0;
    }

    // 5. Hedef Mimariyi Yapılandırma (İlk aşamada RISC-V)
    std::string targetTriple = "riscv64-unknown-elf"; // Veya komut satırından al
    std::string Error;
    auto Target = llvm::TargetRegistry::lookupTarget(targetTriple, Error);
    if (!Target) {
        diagnostics.reportError("Hedef mimari bulunamadı: " + Error);
        diagnostics.printAll();
        return diagnostics.hasErrors() ? 1 : 0;
    }

    llvm::TargetOptions opt;
    auto TargetMachine = std::unique_ptr<llvm::TargetMachine>(
        Target->createTargetMachine(targetTriple, "generic", "", opt,
                                    llvm::Reloc::PIC_, llvm::CodeModel::Default,
                                    optimizationLevel));
    if (!TargetMachine) {
        diagnostics.reportError("Hedef makine oluşturulamadı.");
        diagnostics.printAll();
        return diagnostics.hasErrors() ? 1 : 0;
    }

    // 6. Giriş Kaynak Kodunu Oku
    std::ifstream sourceFile(InputFilename);
    if (!sourceFile.is_open()) {
        diagnostics.reportError("Giriş dosyası açılamadı: " + InputFilename);
        diagnostics.printAll();
        return diagnostics.hasErrors() ? 1 : 0;
    }
    std::string sourceCode((std::istreambuf_iterator<char>(sourceFile)),
                           (std::istreambuf_iterator<char>()));
    sourceFile.close();

    // Giriş dosyasının yolunu al (InterfaceExtractor için gerekli olabilir)
     std::filesystem::path sourceFilePath = InputFilename;
     std::filesystem::path currentDirectory = std::filesystem::current_path(); // Mevcut çalışma dizini


    // 7. İmport Arama Yollarını Ayarla
    moduleResolver.setImportSearchPaths(ImportSearchPaths);
    // Varsayılan arama yollarını da ekleyebilirsiniz (örn: derleyici kurulu olduğu yer, mevcut dizin)
     moduleResolver.addImportSearchPath(currentDirectory.string());


    // 8. Frontend Aşamaları: Lexing, Parsing, Semantic Analysis
    Lexer lexer(sourceCode, InputFilename);
    Parser parser(lexer, diagnostics); // Parser diagnostics'i kullanır
    auto programAST = parser.parse();

    // Parsing hatası varsa durdur
    if (diagnostics.hasErrors()) {
        diagnostics.printAll();
        return diagnostics.hasErrors() ? 1 : 0;
    }

    // Semantik Analiz
    SemanticAnalyzer sema(diagnostics, typeSystem, symbolTable, ownershipChecker, moduleResolver);
    bool semanticErrorsFound = sema.analyze(programAST.get()); // AST'nin pointer'ını geçin

    // Semantik hata varsa durdur
    if (semanticErrorsFound) {
        diagnostics.printAll();
        return diagnostics.hasErrors() ? 1 : 0;
    }

    // 9. Arayüz Çıkarımı ve Kaydetme (.hnt dosyası oluşturma)
    // Semantik analiz başarılıysa arayüzü çıkar ve kaydet
    std::string moduleName = cnt_compiler::getFileNameWithoutExtension(sourceFilePath); // Modül adını dosya adından alalım

    // Çıktı .hnt dosyasının yolunu belirle
    std::filesystem::path outputInterfacePath;
    if (!OutputInterfaceLocation.empty()) {
        std::filesystem::path hntOutputBase = OutputInterfaceLocation;
        if (hntOutputBase.has_extension()) {
            // Eğer uzantı belirtilmişse, kullanıcının tam dosya adı istediğini varsayalım
            outputInterfacePath = hntOutputBase;
        } else {
            // Uzantı belirtilmemişse, bir dizin olduğunu varsayalım ve dosya adını oluşturalım
             outputInterfacePath = hntOutputBase / (moduleName + interfaceExtractor.interfaceFileExtension);
        }
    } else {
        // .hnt çıktı yolu belirtilmemişse, varsayılan olarak mevcut dizin + modül adı.hnt kullan
        outputInterfacePath = currentDirectory / (moduleName + interfaceExtractor.interfaceFileExtension);
        // Veya object dosyasının dizinini kullanabilirsiniz:
         if (!OutputObjectFilename.empty()) {
             outputInterfacePath = std::filesystem::path(OutputObjectFilename).parent_path() / (moduleName + interfaceExtractor.interfaceFileExtension);
         } else {
             outputInterfacePath = currentDirectory / (moduleName + interfaceExtractor.interfaceFileExtension);
         }
    }

    // Arayüzü çıkar
    std::shared_ptr<ModuleInterface> moduleInterface = interfaceExtractor.extract(programAST.get(), moduleName, sourceFilePath);

    if (moduleInterface) {
        // Arayüzü dosyaya kaydet
        if (!interfaceExtractor.save(*moduleInterface, outputInterfacePath)) {
             // Kaydetme hatası zaten diagnostics tarafından raporlandı.
        } else {
             diagnostics.reportInfo("Modül arayüzü başarıyla kaydedildi: " + outputInterfacePath.string());
        }
    } else {
         // Arayüz çıkarımında hata oluştu (SEMA hatası yoksa bu bir internal extractor hatasıdır)
         diagnostics.reportInternalError("Modül arayüzü çıkarılamadı.");
    }


    // Eğer anlamsal hatalar veya arayüz çıkarma/kaydetme hataları varsa, kod üretimine geçme
    if (diagnostics.hasErrors()) {
        diagnostics.printAll();
        return diagnostics.hasErrors() ? 1 : 0;
    }


    // 10. Kod Üretimi (LLVM IR ve Object File)
    llvm::LLVMContext llvmContext;
    LLVMCodeGenerator codeGen(diagnostics, typeSystem, symbolTable, llvmContext, *TargetMachine);
    std::unique_ptr<llvm::Module> llvmModule = codeGen.generate(programAST.get());

    // Kod üretimi hatalarını kontrol et (Code generator genellikle SEMA sonrası hata üretmemeli, ama yine de kontrol iyi olur)
     if (!llvmModule || diagnostics.hasErrors()) {
        diagnostics.reportInternalError("LLVM kod üretimi sırasında hata.");
        diagnostics.printAll();
        return diagnostics.hasErrors() ? 1 : 0;
    }

    // 11. Object Dosyası Oluşturma
    // Çıktı object dosya adını belirle
    std::filesystem::path outputObjectPath;
    if (!OutputObjectFilename.empty()) {
        outputObjectPath = OutputObjectFilename;
    } else {
         // Belirtilmemişse, varsayılan olarak mevcut dizin + modül adı.o kullan
         outputObjectPath = currentDirectory / (moduleName + ".o");
    }


    std::error_code EC;
    // Object dosyasını aç
    llvm::raw_fd_ostream dest(outputObjectPath.string(), EC, llvm::sys::fs::OF_None); // OF_None for binary output
    if (EC) {
        diagnostics.reportError("Çıktı dosyası '" + outputObjectPath.string() + "' açılamadı: " + EC.message());
        diagnostics.printAll();
        return diagnostics.hasErrors() ? 1 : 0;
    }

    // LLVM PassManager'ı kullanarak object file'ı yaz
    llvm::legacy::PassManager PM;
    // TargetMachine'e object file üretimi için gerekli geçişleri ekle
    if (TargetMachine->addPassesToEmitFile(PM, dest, nullptr, llvm::CodeGenFileType::ObjectFile)) {
        diagnostics.reportError("Hedef makine object file üretemiyor.");
        diagnostics.printAll();
        return diagnostics.hasErrors() ? 1 : 0;
    }

    PM.run(*llvmModule); // Geçişleri çalıştır (kod üretimi dahil)
    dest.flush(); // Tamponu boşalt

    // 12. Linkleme (Placeholder)
    // Object dosyalarını çalıştırılabilir dosyaya linkleme aşaması.
    // Genellikle lld gibi harici bir linker çağrılarak yapılır.
    // Bu kısım şimdilik atlanmıştır.
     diagnostics.reportInfo("Object file oluşturuldu: " + outputObjectPath.string());
     diagnostics.reportInfo("Linkleme aşaması atlandı.");


    // 13. Sonuçları Yazdır ve Hata Durumunu Dön
    diagnostics.printAll();

    return diagnostics.hasErrors() ? 1 : 0; // Hata varsa 1, yoksa 0 döndür
}
