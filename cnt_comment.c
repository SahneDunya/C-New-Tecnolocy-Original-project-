#include "cnt_comment.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// İsteğe bağlı: Yorum bilgisi oluşturma fonksiyonu
CommentInfo* comment_info_create(CommentType type, int line, int col, const char* content) {
    CommentInfo* comment = (CommentInfo*)malloc(sizeof(CommentInfo));
    if (!comment) {
        perror("Bellek ayırma hatası");
        exit(EXIT_FAILURE);
    }
    comment->type = type;
    comment->line_number = line;
    comment->column_number = col;
    comment->content = strdup(content);
    return comment;
}

// İsteğe bağlı: Yorum bilgisini serbest bırakma fonksiyonu
void comment_info_free(CommentInfo* comment) {
    if (comment) {
        free(comment->content);
        free(comment);
    }
}