#include "diagnostics.h"
#include <iomanip> // setw için

// Diagnosticseverity enum değerlerini string'e çeviren implementasyon
std::string Diagnostic::getSeverityString() const {
    switch (severity) {
        case DiagnosticSeverity::Info: return "Bilgi";
        case DiagnosticSeverity::Warning: return "Uyari";
        case DiagnosticSeverity::Error: return "HATA";
        case DiagnosticSeverity::InternalError: return "DERLEYICI HATASI";
        default: return "BILINMEYEN";
    }
}

// Diagnostics Kurucu
Diagnostics::Diagnostics() = default; // Varsayılan kurucu yeterli

// Tanı mesajı raporla (konum bilgili)
void Diagnostics::report(DiagnosticSeverity severity, TokenLocation location, const std::string& message) {
    diagnostics.emplace_back(severity, std::move(location), message); // Tanıyı listeye ekle (emplace_back daha verimli olabilir)

    if (severity == DiagnosticSeverity::Error || severity == DiagnosticSeverity::InternalError) {
        errorCount++;
    }

    // Eğer sessiz mod aktif değilse, tanıyı hemen yazdır
    if (!silentMode) {
        printDiagnostic(diagnostics.back()); // Yeni eklenen tanıyı yazdır
    }
}

// Tanı mesajı raporla (konum bilgisiz)
void Diagnostics::report(DiagnosticSeverity severity, const std::string& message) {
    diagnostics.emplace_back(severity, message); // Konum bilgisiz tanıyı listeye ekle

    if (severity == DiagnosticSeverity::Error || severity == DiagnosticSeverity::InternalError) {
        errorCount++;
    }

     if (!silentMode) {
        printDiagnostic(diagnostics.back()); // Yeni eklenen tanıyı yazdır
    }
}

// Farklı seviyeler için kolaylık sağlayan raporlama metodları (konum bilgili)
void Diagnostics::reportInfo(TokenLocation location, const std::string& message) {
    report(DiagnosticSeverity::Info, std::move(location), message);
}

void Diagnostics::reportWarning(TokenLocation location, const std::string& message) {
    report(DiagnosticSeverity::Warning, std::move(location), message);
}

void Diagnostics::reportError(TokenLocation location, const std::string& message) {
    report(DiagnosticSeverity::Error, std::move(location), message);
}

void Diagnostics::reportInternalError(TokenLocation location, const std::string& message) {
    report(DiagnosticSeverity::InternalError, std::move(location), message);
}

// Farklı seviyeler için kolaylık sağlayan raporlama metodları (konum bilgisiz)
void Diagnostics::reportInfo(const std::string& message) {
    report(DiagnosticSeverity::Info, message);
}

void Diagnostics::reportWarning(const std::string& message) {
    report(DiagnosticSeverity::Warning, message);
}

void Diagnostics::reportError(const std::string& message) {
    report(DiagnosticSeverity::Error, message);
}

void Diagnostics::reportInternalError(const std::string& message) {
    report(DiagnosticSeverity::InternalError, message);
}


// Tek bir tanıyı formatlayıp yazdıran yardımcı fonksiyon
void Diagnostics::printDiagnostic(const Diagnostic& diag) const {
    std::ostream& os = std::cerr; // Tanıları genellikle stderr'e yazdırırız

    // Tanı seviyesini yazdır (örn: HATA, Uyari)
    os << std::left << std::setw(15) << diag.getSeverityString() << ": ";

    // Konum bilgisini yazdır (eğer geçerliyse)
    if (diag.location.line != -1) {
        os << diag.location.filename << ":" << diag.location.line << ":" << diag.location.column << ": ";
    } else {
        os << "(Genel): "; // Konum belirtilmemişse
    }

    // Mesajı yazdır
    os << diag.message << std::endl;

    // (Opsiyonel) Eğer kaynak kod snippet'i de gösterecekseniz burada implemente edin.
    // Bu, TokenLocation'ın kaynak kod stringine veya iteratörlerine sahip olmasını gerektirir.
}


// Toplanan tüm tanıları belirlenen çıktı akışına (varsayılan stderr) yazdırır
void Diagnostics::printAll() const {
    // Eğer sıralama aktifse burada çağırabilirsiniz: sortByLocation();
     const_cast<Diagnostics*>(this)->sortByLocation(); // Sıralama non-const metod olduğu için const_cast gerekebilir veya sortByLocation'ı const yapın eğer mümkünse.

    for (const auto& diag : diagnostics) {
        printDiagnostic(diag);
    }
}

// Toplanan tüm tanıları temizler
void Diagnostics::clear() {
    diagnostics.clear();
    errorCount = 0;
}

// Tanıları kaynak kod konumuna göre sıralar
// Implementasyon, TokenLocation'ın karşılaştırılabilir olmasını gerektirir.
void Diagnostics::sortByLocation() {
     std::sort(diagnostics.begin(), diagnostics.end(),
              [](const Diagnostic& a, const Diagnostic& b) {
                  // Önce dosya adına göre sırala
                  if (a.location.filename != b.location.filename) {
                      return a.location.filename < b.location.filename;
                  }
                  // Sonra satır numarasına göre
                  if (a.location.line != b.location.line) {
                      return a.location.line < b.location.line;
                  }
                  // Son olarak sütun numarasına göre
                  return a.location.column < b.location.column;
              });
}
