#ifndef CNT_COMPILER_DIAGNOSTICS_H
#define CNT_COMPILER_DIAGNOSTICS_H

#include "token.h" // Token konum bilgisi (TokenLocation) için

#include <string>
#include <vector>
#include <iostream> // Raporları yazdırmak için
#include <algorithm> // std::sort için (opsiyonel, raporları konuma göre sıralamak isterseniz)
#include <utility> // std::move için

// Tanı türleri (Severity levels)
enum class DiagnosticSeverity {
    Info,         // Bilgilendirme mesajı
    Warning,      // Uyarı (derlemeye devam edilebilir)
    Error,        // Hata (genellikle derlemeyi durdurur veya çalıştırılabilir kod üretilmesini engeller)
    InternalError // Derleyici hatası (olmaması gereken bir durum)
};

// Tek bir tanı mesajını temsil eden yapı
struct Diagnostic {
    DiagnosticSeverity severity; // Tanı türü
    TokenLocation location;      // Sorunun başladığı konum
    std::string message;         // Tanı mesajı

    // Belirli bir konuma sahip tanı için kurucu
    Diagnostic(DiagnosticSeverity s, TokenLocation loc, std::string msg)
        : severity(s), location(std::move(loc)), message(std::move(msg)) {}

    // Belirli bir konuma sahip olmayan tanı için kurucu (Genel hatalar, yapılandırma hataları vb.)
     Diagnostic(DiagnosticSeverity s, std::string msg)
        : severity(s), location(TokenLocation("", -1, -1)), message(std::move(msg)) {} // Geçersiz/belirtilmemiş konum

    // Tanı türünü string olarak döndüren yardımcı fonksiyon
    std::string getSeverityString() const;

    // Tanının bir hata seviyesinde olup olmadığını kontrol et
    bool isError() const {
        return severity == DiagnosticSeverity::Error || severity == DiagnosticSeverity::InternalError;
    }
};


// Tanı Yönetim Sınıfı
class Diagnostics {
private:
    std::vector<Diagnostic> diagnostics; // Toplanan tanılar
    int errorCount = 0;                 // Tespit edilen hata sayısı (Error ve InternalError)
    bool silentMode = false;           // True ise tanılar hemen yazılmaz, sadece toplanır.

    // Tek bir tanıyı formatlayıp yazdıran yardımcı fonksiyon
    void printDiagnostic(const Diagnostic& diag) const;

public:
    // Kurucu
    Diagnostics();

    // Tanı mesajı raporla (konum bilgili)
    void report(DiagnosticSeverity severity, TokenLocation location, const std::string& message);
    // Tanı mesajı raporla (konum bilgisiz)
    void report(DiagnosticSeverity severity, const std::string& message);


    // Farklı seviyeler için kolaylık sağlayan raporlama metodları (konum bilgili)
    void reportInfo(TokenLocation location, const std::string& message);
    void reportWarning(TokenLocation location, const std::string& message);
    void reportError(TokenLocation location, const std::string& message);
    void reportInternalError(TokenLocation location, const std::string& message);

    // Farklı seviyeler için kolaylık sağlayan raporlama metodları (konum bilgisiz)
    void reportInfo(const std::string& message);
    void reportWarning(const std::string& message);
    void reportError(const std::string& message);
    void reportInternalError(const std::string& message);


    // Toplanan tüm tanıları belirlenen çıktı akışına (varsayılan stderr) yazdırır
    void printAll() const;

    // Herhangi bir hata (Error veya InternalError) raporlanıp raporlanmadığını kontrol eder
    bool hasErrors() const { return errorCount > 0; }

    // Raporlanan toplam hata sayısını döndürür
    int getErrorCount() const { return errorCount; }

    // Toplanan tüm tanıları temizler (Örn: birden fazla dosya derlenirken)
    void clear();

    // Sessiz modu ayarlar (raporlar sadece toplanır, hemen yazılmaz)
    void setSilentMode(bool silent) { silentMode = silent; }
    bool isSilentMode() const { return silentMode; }

    // (Opsiyonel) Tanıları kaynak kod konumuna göre sıralar (printAll öncesi çağrılabilir)
    void sortByLocation();
};

#endif // CNT_COMPILER_DIAGNOSTICS_H
