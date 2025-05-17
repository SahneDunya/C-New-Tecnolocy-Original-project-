#ifndef CNT_COMPILER_TOKEN_H
#define CNT_COMPILER_TOKEN_H

#include <string>
#include <utility> // For std::move

// Token türlerini temsil eden enum
// Dilinizdeki tüm anahtar kelimeler, operatörler, noktalama işaretleri vb. buraya eklenecek
struct Token {
    enum Type {
        // Özel Tokenler
        TOK_EOF,    // Dosya Sonu
        TOK_ERROR,  // Lexing hatası

        // Tanımlayıcılar
        TOK_IDENTIFIER,

        // Değişmezler (Literals)
        TOK_INT_LITERAL,    // Tam sayılar (örn: 123)
        TOK_FLOAT_LITERAL,  // Ondalıklı sayılar (örn: 1.23, 1e5)
        TOK_STRING_LITERAL, // Dizgiler (örn: "merhaba")
        TOK_CHAR_LITERAL,   // Karakterler (örn: 'a')

        // Anahtar Kelimeler (CNT diline özel anahtar kelimeler)
        TOK_IMPORT, TOK_FN, TOK_STRUCT, TOK_ENUM, TOK_LET, TOK_MUT,
        TOK_IF, TOK_ELSE, TOK_WHILE, TOK_FOR, TOK_RETURN, TOK_BREAK, TOK_CONTINUE,
        TOK_MATCH, // Sadece match desteklendiği için bu önemli bir anahtar kelime
        TOK_TRUE, TOK_FALSE, // Boolean değişmezleri
        // ... eklenecek diğer anahtar kelimeler (pub, use, as, impl, trait, etc.)

        // Operatörler
        TOK_ASSIGN,     // =
        TOK_PLUS,       // +
        TOK_MINUS,      // -
        TOK_STAR,       // *
        TOK_SLASH,      // /
        TOK_PERCENT,    // % (Modül)
        TOK_EQ,         // == (Eşitlik)
        TOK_NE,         // != (Eşit değil)
        TOK_LT,         // < (Küçük)
        TOK_GT,         // > (Büyük)
        TOK_LE,         // <= (Küçük eşit)
        TOK_GE,         // >= (Büyük eşit)
        TOK_AND,        // && (Mantıksal VE)
        TOK_OR,         // || (Mantıksal VEYA)
        TOK_NOT,        // ! (Mantıksal DEĞİL)
        // Atama Operatörleri (İsteğe bağlı, C++ benzeri: +=, -=, *=, vb.)
        TOK_PLUS_ASSIGN, // +=
        // ... eklenecek diğer operatörler

        // Noktalama İşaretleri
        TOK_LPAREN,     // (
        TOK_RPAREN,     // )
        TOK_LBRACE,     // {
        TOK_RBRACE,     // }
        TOK_LBRACKET,   // [
        TOK_RBRACKET,   // ]
        TOK_COMMA,      // ,
        TOK_SEMICOLON,  // ;
        TOK_COLON,      // :
        TOK_DOT,        // .
        TOK_ARROW,      // -> (İşaretçi veya dönüş türü gösterimi)
        TOK_FAT_ARROW,  // => (Match kollarında veya lambda'larda)
        TOK_DOUBLE_COLON, // :: (Yol ayırıcı veya ilişkilendirilmiş fonksiyonlar için)
        // ... eklenecek diğer noktalama işaretleri

        // Eğer header dosyaları (.hnt) otomatik çıkarıma rağmen bir rol oynayacaksa
        // TOK_HNT_INCLUDE, // Eğer bir tür include direktifi varsa
        // ...
    };

    Type type;          // Token türü
    std::string lexeme; // Kaynak kodundaki token'a karşılık gelen metin (lexeme)
    int line;           // Token'ın bulunduğu satır numarası (1 tabanlı)
    int column;         // Token'ın başladığı sütun numarası (1 tabanlı)
    std::string filename; // Token'ın bulunduğu dosya adı

    // Kurucu (Constructor)
    Token(Type t, std::string lex, int l, int c, std::string fn)
        : type(t), lexeme(std::move(lex)), line(l), column(c), filename(std::move(fn)) {}

    // Hata veya dosya sonu gibi lexeme'i olmayan tokenler için kurucu
    Token(Type t, int l, int c, std::string fn)
        : type(t), line(l), column(c), filename(std::move(fn)) {}

    // Token türünü string olarak döndüren yardımcı fonksiyon (Hata ayıklama için kullanışlı)
    std::string getTypeString() const;
};

#endif // CNT_COMPILER_TOKEN_H
