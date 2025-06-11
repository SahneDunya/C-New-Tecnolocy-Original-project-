#ifndef CNT_COMPILER_LEXER_H
#define CNT_COMPILER_LEXER_H

#include <string>
#include <unordered_map> // Anahtar kelimeler için
#include "token.h"       // Token tanımı
#include "diagnostics.h" // Hata raporlama sistemi (main.cpp'de kullandık, burada da lazım)

// Lexer sınıfı
class Lexer {
private:
    std::string sourceCode; // Derlenecek kaynak kod
    std::string filename;   // Kaynak dosya adı
    int currentPos;         // Kaynak kod stringindeki o anki pozisyon
    int currentLine;        // O anki satır numarası
    int currentColumn;      // O anki sütun numarası

    // Anahtar kelimeleri tutacak harita
    std::unordered_map<std::string, Token::Type> keywords;

    // Yardımcı fonksiyonlar
    char peek();       // Sonraki karakteri okumadan bakar
    char consume();    // O anki karakteri okur ve pozisyonu ilerletir
    bool isAtEnd();    // Kaynak kodun sonuna ulaşıldı mı?

    void skipWhitespaceAndComments(); // Boşlukları ve yorumları atla
    void skipComment();               // Yorumları atla (Tek satır veya çoklu satır - CNT'de yorum sözdizimini belirleyin)

    Token readIdentifierOrKeyword(); // Tanımlayıcıyı veya anahtar kelimeyi oku
    Token readNumber();              // Sayısal değişmezi (int veya float) oku
    Token readString();              // String değişmezini oku
    Token readChar();                // Karakter değişmezini oku
    Token readOperatorOrPunctuation(); // Operatörü veya noktalama işaretini oku

    // Hata raporlama (Diagnostics sistemini kullanır)
    void reportLexingError(const std::string& message);

    // Token'ın başladığı pozisyonu kaydetmek için
    int startColumn;
    int startLine;

public:
    Lexer(std::string source, std::string fname = "input"); // Kurucu
    Token getNextToken(); // Sonraki token'ı döndüren ana metod

    // Parser tarafından hata raporlama için kullanılabilir
    int getCurrentLine() const { return currentLine; }
    int getCurrentColumn() const { return currentColumn; }
    std::string getFilename() const { return filename; }
};

#endif // CNT_COMPILER_LEXER_H
