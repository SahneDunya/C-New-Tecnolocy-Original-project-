#ifndef CNT_COMMENT_H
#define CNT_COMMENT_H

#include <stdbool.h>

// Yorum Türleri
typedef enum {
    COMMENT_SINGLE_LINE, // Tek satırlık yorum (örneğin, // ...)
    COMMENT_MULTI_LINE_START, // Çok satırlık yorumun başlangıcı (örneğin, /*)
    COMMENT_MULTI_LINE_END,   // Çok satırlık yorumun sonu (örneğin, */)
    COMMENT_DOCUMENTATION     // Belgeleme amaçlı özel yorumlar (isteğe bağlı)
} CommentType;

// Yorum bilgisini tutabilecek bir yapı (isteğe bağlı)
typedef struct CommentInfo {
    CommentType type;
    int line_number;
    int column_number;
    char* content;
} CommentInfo;

#endif