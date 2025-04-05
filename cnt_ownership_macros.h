#ifndef CNT_OWNERSHIP_MACROS_H
#define CNT_OWNERSHIP_MACROS_H

#include "cnt_ownership.h"

// Sahiplik Oluşturma Makrosu
#define OWNERSHIP_CREATE(value) ownership_create(value)

// Sahiplik Devretme Makrosu
#define OWNERSHIP_MOVE(owner) ownership_move(owner)

// Sahiplik Ödünç Alma Makrosu
#define OWNERSHIP_BORROW(owner) ownership_borrow(owner)

// Sahiplik Değiştirilebilir Ödünç Alma Makrosu
#define OWNERSHIP_BORROW_MUT(owner) ownership_borrow_mut(owner)

// Sahiplik Serbest Bırakma Makrosu
#define OWNERSHIP_DROP(owner) ownership_drop(owner)

#endif