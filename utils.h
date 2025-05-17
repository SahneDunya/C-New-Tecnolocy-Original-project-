#ifndef CNT_COMPILER_UTILS_H
#define CNT_COMPILER_UTILS_H

#include <string>
#include <vector>
#include <iostream> // Akış (stream) işlemleri için (örn: yazdırma)
#include <filesystem> // Dosya sistemi yolları için (C++17)
#include <algorithm> // std::all_of, std::transform vb. için
#include <cctype>    // std::isspace için

// Diğer bileşenlerin başlıkları (Eğer yardımcı fonksiyonlar bu tipler üzerinde çalışacaksa)
#include "token.h" // TokenLocation için
#include "ast.h"   // Temel ASTNode ve diğer AST düğümleri için (eğer print fonksiyonları AST'yi gezecekse)
#include "type_system.h" // Type* için (eğer tip yazdırma fonksiyonları olacaksa)


// CNT Derleyiciye özgü yardımcılar için isim alanı
namespace cnt_compiler {

    // =======================================================================
    // String Yardımcıları
    // =======================================================================

    // String'in başındaki ve sonundaki boşlukları kırpar
    std::string trim(const std::string& str);

    // String'i belirli bir ayırıcıya göre parçalara ayırır
    std::vector<std::string> splitString(const std::string& str, const std::string& delimiter);

    // String vektörünü belirli bir ayırıcı ile birleştirir
    std::string joinStrings(const std::vector<std::string>& strings, const std::string& delimiter);

    // String'i küçük harfe dönüştürür
    std::string toLower(const std::string& str);

    // String'i büyük harfe dönüştürür
    std::string toUpper(const std::string& str);


    // =======================================================================
    // Konum ve Hata Ayıklama Yardımcıları
    // =======================================================================

    // TokenLocation bilgisini formatlı bir string olarak döndürür (örn: "file.cnt:10:5")
    std::string getLocationString(const TokenLocation& loc);

    // Bir token'ı çıktı akışına yazdırır (hata ayıklama için)
    void printToken(std::ostream& os, const Token& token);

    // Bir AST düğümünü ve alt düğümlerini çıktı akışına yazdırır (hata ayıklama için)
    // Bu, AST yapısını görselleştirmeye yardımcı olur.
    void printASTNode(std::ostream& os, const ASTNode* node, int indent = 0);

    // (Opsiyonel) Bir semantik Type* objesini çıktı akışına yazdırır (Type::toString kullanır)
     void printType(std::ostream& os, const Type* type);


    // =======================================================================
    // Dosya Sistemi Yardımcıları
    // =======================================================================

    // Bir dosya yolunun uzantısını döndürür (nokta dahil, örn: ".txt")
    std::string getFileExtension(const std::filesystem::path& path);

    // Bir dosya yolunun uzantısı olmayan dosya adını döndürür (örn: "path/to/file.txt" -> "file")
    std::string getFileNameWithoutExtension(const std::filesystem::path& path);

    // Belirtilen dosya yolunun var olup olmadığını kontrol eder
    bool fileExists(const std::filesystem::path& path);

    // Belirtilen yolu normalize eder (./, ../ gibi kısımları çözer)
    std::filesystem::path normalizePath(const std::filesystem::path& path);


    // =======================================================================
    // Diğer Genel Yardımcılar
    // =======================================================================

    // Eğer enum'ları string'e çevirme gibi sık kullanılan bir işlev varsa buraya eklenebilir.
    // Token türünü string olarak döndürmek gibi (şu anda Token sınıfında var).

} // namespace cnt_compiler

#endif // CNT_COMPILER_UTILS_H
