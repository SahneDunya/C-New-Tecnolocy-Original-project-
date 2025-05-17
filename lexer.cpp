#include "lexer.h"
#include <cctype>      // isalpha, isdigit, isalnum vb. için
#include <iostream>    // Hata ayıklama çıktısı için (geçici olabilir)
#include <string>      // String işlemleri için

// Token türünü string olarak döndüren yardımcı fonksiyon implementasyonu
std::string Token::getTypeString() const {
    switch (type) {
        case TOK_EOF: return "TOK_EOF";
        case TOK_ERROR: return "TOK_ERROR";
        case TOK_IDENTIFIER: return "TOK_IDENTIFIER";
        case TOK_INT_LITERAL: return "TOK_INT_LITERAL";
        case TOK_FLOAT_LITERAL: return "TOK_FLOAT_LITERAL";
        case TOK_STRING_LITERAL: return "TOK_STRING_LITERAL";
        case TOK_CHAR_LITERAL: return "TOK_CHAR_LITERAL";
        case TOK_IMPORT: return "TOK_IMPORT";
        case TOK_FN: return "TOK_FN";
        case TOK_STRUCT: return "TOK_STRUCT";
        case TOK_ENUM: return "TOK_ENUM";
        case TOK_LET: return "TOK_LET";
        case TOK_MUT: return "TOK_MUT";
        case TOK_IF: return "TOK_IF";
        case TOK_ELSE: return "TOK_ELSE";
        case TOK_WHILE: return "TOK_WHILE";
        case TOK_FOR: return "TOK_FOR";
        case TOK_RETURN: return "TOK_RETURN";
        case TOK_BREAK: return "TOK_BREAK";
        case TOK_CONTINUE: return "TOK_CONTINUE";
        case TOK_MATCH: return "TOK_MATCH";
        case TOK_TRUE: return "TOK_TRUE";
        case TOK_FALSE: return "TOK_FALSE";
        case TOK_ASSIGN: return "TOK_ASSIGN";
        case TOK_PLUS: return "TOK_PLUS";
        case TOK_MINUS: return "TOK_MINUS";
        case TOK_STAR: return "TOK_STAR";
        case TOK_SLASH: return "TOK_SLASH";
        case TOK_PERCENT: return "TOK_PERCENT";
        case TOK_EQ: return "TOK_EQ";
        case TOK_NE: return "TOK_NE";
        case TOK_LT: return "TOK_LT";
        case TOK_GT: return "TOK_GT";
        case TOK_LE: return "TOK_LE";
        case TOK_GE: return "TOK_GE";
        case TOK_AND: return "TOK_AND";
        case TOK_OR: return "TOK_OR";
        case TOK_NOT: return "TOK_NOT";
        case TOK_PLUS_ASSIGN: return "TOK_PLUS_ASSIGN";
        case TOK_LPAREN: return "TOK_LPAREN";
        case TOK_RPAREN: return "TOK_RPAREN";
        case TOK_LBRACE: return "TOK_LBRACE";
        case TOK_RBRACE: return "TOK_RBRACE";
        case TOK_LBRACKET: return "TOK_LBRACKET";
        case TOK_RBRACKET: return "TOK_RBRACKET";
        case TOK_COMMA: return "TOK_COMMA";
        case TOK_SEMICOLON: return "TOK_SEMICOLON";
        case TOK_COLON: return "TOK_COLON";
        case TOK_DOT: return "TOK_DOT";
        case TOK_ARROW: return "TOK_ARROW";
        case TOK_FAT_ARROW: return "TOK_FAT_ARROW";
        case TOK_DOUBLE_COLON: return "TOK_DOUBLE_COLON";
        // ... diğer token türleri
        default: return "UNKNOWN_TOKEN_TYPE";
    }
}

// Lexer Kurucu
Lexer::Lexer(std::string source, std::string fname)
    : sourceCode(std::move(source)), filename(std::move(fname)),
      currentPos(0), currentLine(1), currentColumn(1),
      startLine(1), startColumn(1) {

    // Anahtar kelime haritasını doldur
    keywords["import"] = Token::TOK_IMPORT;
    keywords["fn"] = Token::TOK_FN;
    keywords["struct"] = Token::TOK_STRUCT;
    keywords["enum"] = Token::TOK_ENUM;
    keywords["let"] = Token::TOK_LET;
    keywords["mut"] = Token::TOK_MUT;
    keywords["if"] = Token::TOK_IF;
    keywords["else"] = Token::TOK_ELSE;
    keywords["while"] = Token::TOK_WHILE;
    keywords["for"] = Token::TOK_FOR;
    keywords["return"] = Token::TOK_RETURN;
    keywords["break"] = Token::TOK_BREAK;
    keywords["continue"] = Token::TOK_CONTINUE;
    keywords["match"] = Token::TOK_MATCH;
    keywords["true"] = Token::TOK_TRUE;
    keywords["false"] = Token::TOK_FALSE;
    // ... diğer anahtar kelimeleri ekle
}

// Sonraki karakteri okumadan bakar
char Lexer::peek() {
    if (isAtEnd()) {
        return '\0'; // Dosya sonu işareti
    }
    return sourceCode[currentPos];
}

// Sonraki karakterden sonraki karaktere bakar
char Lexer::peekNext() {
     if (currentPos + 1 >= sourceCode.length()) {
        return '\0'; // Dosya sonu işareti
    }
    return sourceCode[currentPos + 1];
}

// O anki karakteri okur ve pozisyonu ilerletir
char Lexer::consume() {
    if (isAtEnd()) {
        return '\0';
    }
    char c = sourceCode[currentPos++];
    if (c == '\n') {
        currentLine++;
        currentColumn = 1;
    } else {
        currentColumn++;
    }
    return c;
}

// Kaynak kodun sonuna ulaşıldı mı?
bool Lexer::isAtEnd() {
    return currentPos >= sourceCode.length();
}

// Hata raporlama (Diagnostics sistemini kullanır)
void Lexer::reportLexingError(const std::string& message) {
    // Diagnostics sistemine hata bildir
    ReportError(filename, startLine, startColumn, message);
}

// Boşlukları ve yorumları atla
void Lexer::skipWhitespaceAndComments() {
    while (true) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            consume(); // Boşluk karakterlerini tüket
        } else if (c == '/') {
            if (peekNext() == '/') {
                // Tek satırlık yorum (//...)
                skipComment();
            } else if (peekNext() == '*') {
                 // Çok satırlık yorum (/* ... */)
                 skipComment(); // Yorum atlama fonksiyonunu çok satırlıyı da içerecek şekilde genişletin
            }
            else {
                break; // Yorum değil, operatör olabilir
            }
        }
        else {
            break; // Boşluk veya yorum değil
        }
    }
}

// Yorumları atla (Tek satır // ve Çok satır /* */ desteklediğimizi varsayalım)
void Lexer::skipComment() {
    consume(); // '/' karakterini tüket

    if (peek() == '/') {
        // Tek satırlık yorum (//)
        consume(); // İkinci '/' karakterini tüket
        while (peek() != '\n' && !isAtEnd()) {
            consume(); // Satır sonuna veya dosya sonuna kadar karakterleri tüket
        }
         // Satır sonunu (newline) tüket, böylece next getNextToken() doğru satırdan başlar
        if (peek() == '\n') {
            consume();
        }

    } else if (peek() == '*') {
        // Çok satırlık yorum (/* */)
        consume(); // '*' karakterini tüket
        while (!isAtEnd()) {
            if (peek() == '*' && peekNext() == '/') {
                consume(); // '*' tüket
                consume(); // '/' tüket
                break; // Yorum sonu
            }
            consume(); // Yorum içindeki karakterleri tüket
        }
        if (isAtEnd()) {
            reportLexingError("Yorum kapatılmamış (/* ... */)");
        }
    }
    // Diğer durumlar (sadece '/') skipWhitespaceAndComments tarafından ele alınır.
}


// Tanımlayıcıyı veya anahtar kelimeyi oku
Token Lexer::readIdentifierOrKeyword() {
    startLine = currentLine;
    startColumn = currentColumn;
    std::string value;
    while (std::isalnum(peek()) || peek() == '_') {
        value += consume();
    }

    // Okunan değer bir anahtar kelime mi kontrol et
    if (keywords.count(value)) {
        return Token(keywords[value], value, startLine, startColumn, filename);
    } else {
        return Token(Token::TOK_IDENTIFIER, value, startLine, startColumn, filename);
    }
}

// Sayısal değişmezi (int veya float) oku
Token Lexer::readNumber() {
    startLine = currentLine;
    startColumn = currentColumn;
    std::string value;
    bool isFloat = false;

    while (std::isdigit(peek())) {
        value += consume();
    }

    // Ondalıklı kısım var mı?
    if (peek() == '.' && std::isdigit(peekNext())) {
        isFloat = true;
        value += consume(); // '.' karakterini tüket
        while (std::isdigit(peek())) {
            value += consume();
        }
    }

    // Üstel kısım var mı? (e veya E)
    if ((peek() == 'e' || peek() == 'E') && (std::isdigit(peekNext()) || peekNext() == '+' || peekNext() == '-')) {
         if (peekNext() == '+' || peekNext() == '-') {
             value += consume(); // 'e' veya 'E' tüket
             value += consume(); // '+' veya '-' tüket
         } else {
             value += consume(); // 'e' veya 'E' tüket
         }

         while (std::isdigit(peek())) {
             value += consume();
         }
         isFloat = true; // Üstel gösterim varsa kesin float'tır
    }
    // Not: Daha gelişmiş sayılar için (hex, binary, underscore ayırıcılar gibi)
    // burası genişletilmelidir.

    if (isFloat) {
        return Token(Token::TOK_FLOAT_LITERAL, value, startLine, startColumn, filename);
    } else {
        // Tam sayı için ek kontrol yapılabilir (örn: çok büyük sayı hatası)
        return Token(Token::TOK_INT_LITERAL, value, startLine, startColumn, filename);
    }
}

// String değişmezini oku (Çift tırnak ")
Token Lexer::readString() {
    startLine = currentLine;
    startColumn = currentColumn;
    std::string value; // Tırnaklar hariç string içeriği
    consume(); // Açılış tırnağını (") tüket

    while (peek() != '"' && !isAtEnd()) {
        // Escape karakterlerini ele al (örn: \n, \\, \")
        if (peek() == '\\') {
            consume(); // '\' karakterini tüket
            char escaped_char = consume();
            switch (escaped_char) {
                case 'n': value += '\n'; break;
                case 't': value += '\t'; break;
                case 'r': value += '\r'; break;
                case '"': value += '"'; break;
                case '\\': value += '\\'; break;
                // ... diğer escape sequence'ler (\0, \uXXXX vb.)
                default:
                    reportLexingError("Geçersiz escape sequence '\\" + std::string(1, escaped_char) + "'.");
                    // Hata sonrası ne yapılacağı derleyicinin politikasına bağlıdır.
                    // Şimdilik devam edelim, ama hata bildirildi.
                    value += escaped_char; // Geçersiz karakteri ekle
                    break;
            }
        } else {
            value += consume(); // Normal karakteri ekle
        }
    }

    if (isAtEnd()) {
        reportLexingError("String kapatılmamış.");
        // TOK_ERROR döndürebilir veya eksik string ile devam edebilir.
        // Eksik string ile devam etmek parser için genellikle daha kolaydır.
        return Token(Token::TOK_STRING_LITERAL, value, startLine, startColumn, filename);
    }

    consume(); // Kapanış tırnağını (") tüket
    return Token(Token::TOK_STRING_LITERAL, value, startLine, startColumn, filename);
}

// Karakter değişmezini oku (Tek tırnak ')
Token Lexer::readChar() {
    startLine = currentLine;
    startColumn = currentColumn;
    std::string value; // Tırnaklar hariç karakter içeriği
    consume(); // Açılış tırnağını (') tüket

    if (isAtEnd()) {
        reportLexingError("Karakter kapatılmamış.");
        return Token(Token::TOK_ERROR, startLine, startColumn, filename); // Hata token'ı dön
    }

    if (peek() == '\\') {
         // Escape karakterini ele al
        consume(); // '\' karakterini tüket
        char escaped_char = consume();
        switch (escaped_char) {
            case 'n': value += '\n'; break;
            case 't': value += '\t'; break;
            case 'r': value += '\r'; break;
            case '\'': value += '\''; break;
            case '\\': value += '\\'; break;
            // ... diğer escape sequence'ler (\0, \uXXXX vb.)
            default:
                reportLexingError("Geçersiz escape sequence '\\" + std::string(1, escaped_char) + "'.");
                 // Hata sonrası ne yapılacağı derleyicinin politikasına bağlıdır.
                value += escaped_char; // Geçersiz karakteri ekle
                break;
        }
    } else {
        value += consume(); // Normal karakteri ekle
    }

    if (peek() != '\'') {
        reportLexingError("Karakter değişmezi tek karakter veya escape sequence olmalı ve tek tırnak (') ile kapatılmalı.");
        // Hata sonrası ne yapılacağı derleyicinin politikasına bağlı
        // Şimdilik Hata token'ı dönelim veya eksik tırnakla devam edelim.
        // Hata token'ı parser'ın işini kolaylaştırabilir.
         while (peek() != '\'' && !isAtEnd() && peek() != '\n') { // Kapanış tırnağı veya satır sonu bulana kadar tüket
             consume();
         }
         if (peek() == '\'') consume(); // Kapanış tırnağını tüket
         return Token(Token::TOK_ERROR, startLine, startColumn, filename); // Hata token'ı dön

    }

    consume(); // Kapanış tırnağını (') tüket

    if (value.length() != 1) {
         // Tırnaklar arasında bir karakterden fazla veya escape sequence dışında bir şey varsa hata
         reportLexingError("Karakter değişmezi yalnızca bir karakter veya geçerli bir escape sequence içermeli.");
         return Token(Token::TOK_ERROR, startLine, startColumn, filename); // Hata token'ı dön
    }


    return Token(Token::TOK_CHAR_LITERAL, value, startLine, startColumn, filename);
}


// Operatörü veya noktalama işaretini oku
Token Lexer::readOperatorOrPunctuation() {
    startLine = currentLine;
    startColumn = currentColumn;

    char c = consume(); // İlk karakteri tüket

    switch (c) {
        case '(': return Token(Token::TOK_LPAREN, "(", startLine, startColumn, filename);
        case ')': return Token(Token::TOK_RPAREN, ")", startLine, startColumn, filename);
        case '{': return Token(Token::TOK_LBRACE, "{", startLine, startColumn, filename);
        case '}': return Token(Token::TOK_RBRACE, "}", startLine, startColumn, filename);
        case '[': return Token(Token::TOK_LBRACKET, "[", startLine, startColumn, filename);
        case ']': return Token(Token::TOK_RBRACKET, "]", startLine, startColumn, filename);
        case ',': return Token(Token::TOK_COMMA, ",", startLine, startColumn, filename);
        case ';': return Token(Token::TOK_SEMICOLON, ";", startLine, startColumn, filename);
        case '.': return Token(Token::TOK_DOT, ".", startLine, startColumn, filename);
        case '%': return Token(Token::TOK_PERCENT, "%", startLine, startColumn, filename);

        case '+':
            if (peek() == '=') { consume(); return Token(Token::TOK_PLUS_ASSIGN, "+=", startLine, startColumn, filename); }
            return Token(Token::TOK_PLUS, "+", startLine, startColumn, filename);
        case '-':
            if (peek() == '>') { consume(); return Token(Token::TOK_ARROW, "->", startLine, startColumn, filename); }
             if (peek() == '=') { consume(); return Token(Token::TOK_MINUS_ASSIGN, "-=", startLine, startColumn, filename); } // Eğer destekleniyorsa
            return Token(Token::TOK_MINUS, "-", startLine, startColumn, filename);
        case '*':
             if (peek() == '=') { consume(); return Token(Token::TOK_STAR_ASSIGN, "*=", startLine, startColumn, filename); } // Eğer destekleniyorsa
            return Token(Token::TOK_STAR, "*", startLine, startColumn, filename);
        case '/':
             if (peek() == '=') { consume(); return Token(Token::TOK_SLASH_ASSIGN, "/=", startLine, startColumn, filename); } // Eğer destekleniyorsa
            // Yorumlar skipWhitespaceAndComments tarafından atlandığı için buraya gelmez.
            return Token(Token::TOK_SLASH, "/", startLine, startColumn, filename);
        case '=':
            if (peek() == '=') { consume(); return Token(Token::TOK_EQ, "==", startLine, startColumn, filename); }
            if (peek() == '>') { consume(); return Token(Token::TOK_FAT_ARROW, "=>", startLine, startColumn, filename); }
            return Token(Token::TOK_ASSIGN, "=", startLine, startColumn, filename);
        case '!':
            if (peek() == '=') { consume(); return Token(Token::TOK_NE, "!=", startLine, startColumn, filename); }
            return Token(Token::TOK_NOT, "!", startLine, startColumn, filename);
        case '<':
            if (peek() == '=') { consume(); return Token(Token::TOK_LE, "<=", startLine, startColumn, filename); }
            return Token(Token::TOK_LT, "<", startLine, startColumn, filename);
        case '>':
            if (peek() == '=') { consume(); return Token(Token::TOK_GE, ">=", startLine, startColumn, filename); }
            return Token(Token::TOK_GT, ">", startLine, startColumn, filename);
        case '&':
            if (peek() == '&') { consume(); return Token(Token::TOK_AND, "&&", startLine, startColumn, filename); }
            // Tek '&' karakteri referans veya bitwise AND operatörü olabilir.
            // CNT'nin sözdizimine göre burayı genişletin. Şimdilik sadece && var sayalım.
            reportLexingError("Tanımsız tek '&' karakteri. Belki '&&' mi demek istediniz?");
            return Token(Token::TOK_ERROR, startLine, startColumn, filename);
        case '|':
            if (peek() == '|') { consume(); return Token(Token::TOK_OR, "||", startLine, startColumn, filename); }
            // Tek '|' karakteri bitwise OR veya closure argüman ayırıcısı olabilir.
            // CNT'nin sözdizimine göre burayı genişletin. Şimdilik sadece || var sayalım.
             reportLexingError("Tanımsız tek '|' karakteri. Belki '||' mi demek istediniz?");
            return Token(Token::TOK_ERROR, startLine, startColumn, filename);
        case ':':
            if (peek() == ':') { consume(); return Token(Token::TOK_DOUBLE_COLON, "::", startLine, startColumn, filename); }
            return Token(Token::TOK_COLON, ":", startLine, startColumn, filename);


        default:
            // Tanımsız karakter
            reportLexingError("Tanımsız karakter '" + std::string(1, c) + "'.");
            return Token(Token::TOK_ERROR, startLine, startColumn, filename); // Hata token'ı dön
    }
}


// Sonraki token'ı döndüren ana metod
Token Lexer::getNextToken() {
    // Önce boşlukları ve yorumları atla
    skipWhitespaceAndComments();

    // Dosya sonuna ulaşıldıysa TOK_EOF döndür
    if (isAtEnd()) {
        return Token(Token::TOK_EOF, currentLine, currentColumn, filename);
    }

    // Token'ın başladığı pozisyonu kaydet
    startLine = currentLine;
    startColumn = currentColumn;

    // O anki karaktere bak ve token türünü belirle
    char c = peek();

    if (std::isalpha(c) || c == '_') {
        // Tanımlayıcı veya Anahtar Kelime
        return readIdentifierOrKeyword();
    }

    if (std::isdigit(c)) {
        // Sayısal Değişmez (Tam Sayı veya Ondalıklı)
        return readNumber();
    }

    if (c == '"') {
        // String Değişmez
        return readString();
    }

    if (c == '\'') {
        // Karakter Değişmez
        return readChar();
    }

    // Operatör veya Noktalama İşareti
    // Bu fonksiyon tek veya çok karakterli operatörleri/noktalama işaretlerini ele alacak
    return readOperatorOrPunctuation();

    // Eğer yukarıdaki durumların hiçbiri eşleşmezse, bu tanımsız bir karakterdir
    // Bu durum aslında readOperatorOrPunctuation() içindeki default case tarafından yakalanacaktır.
    // Ancak burada da bir yakalama ekleyebiliriz, defensive programlama için.
     consume(); // Tanımsız karakteri tüket
     reportLexingError("Tanımsız karakter."); // Hata raporla
     return Token(Token::TOK_ERROR, startLine, startColumn, filename); // Hata token'ı dön
}
