#ifndef CNT_OWNERSHIP_ERRORS_H
#define CNT_OWNERSHIP_ERRORS_H

// Sahiplik Hata Kodları
#define OWNERSHIP_ERROR_ALREADY_MOVED 1
#define OWNERSHIP_ERROR_ALREADY_DROPPED 2
#define OWNERSHIP_ERROR_MUTABLE_BORROW_CONFLICT 3
#define OWNERSHIP_ERROR_LIFETIME_EXCEEDED 4
// ... diğer hata kodları ...

// Sahiplik Hata Mesajları
#define OWNERSHIP_ERROR_ALREADY_MOVED_MESSAGE "Sahiplik zaten devredilmiş."
#define OWNERSHIP_ERROR_ALREADY_DROPPED_MESSAGE "Sahiplik zaten serbest bırakılmış."
#define OWNERSHIP_ERROR_MUTABLE_BORROW_CONFLICT_MESSAGE "Aynı anda birden fazla değiştirilebilir ödünç alma hatası."
#define OWNERSHIP_ERROR_LIFETIME_EXCEEDED_MESSAGE "Ödünç alınan referansın yaşam süresi aşıldı."
// ... diğer hata mesajları ...

#endif