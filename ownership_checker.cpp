#include "ownership_checker.h"
#include <cassert> // assert için
#include <algorithm> // std::remove_if için

namespace cnt_compiler { // Derleyici yardımcıları için isim alanı

    // VariableStatus enum değerini string olarak döndüren yardımcı fonksiyon
    std::string getVariableStatusString(VariableStatus status) {
        switch (status) {
            case VariableStatus::Owned: return "Owned";
            case VariableStatus::Moved: return "Moved";
            case VariableStatus::BorrowedImmutable: return "BorrowedImmutable";
            case VariableStatus::BorrowedMutable: return "BorrowedMutable";
            case VariableStatus::Dropped: return "Dropped";
            case VariableStatus::Uninitialized: return "Uninitialized";
            // case VariableStatus::PartiallyMoved: return "PartiallyMoved";
            default: return "UnknownStatus";
        }
    }

    // UseKind enum değerini string olarak döndüren yardımcı fonksiyon
     std::string getUseKindString(UseKind kind) {
        switch (kind) {
            case UseKind::Read: return "Read";
            case UseKind::Write: return "Write";
            case UseKind::Move: return "Move";
            case UseKind::BorrowImmutable: return "BorrowImmutable";
            case UseKind::BorrowMutable: return "BorrowMutable";
            default: return "UnknownUse";
        }
    }


    // Kurucu
    OwnershipChecker::OwnershipChecker(Diagnostics& diag, TypeSystem& ts, SymbolTable& st)
        : diagnostics(diag), typeSystem(ts), symbolTable(st) {
        // Kurulum işlemleri
    }

    // Bir tipin Copy trait'ini implemente edip etmediğini kontrol et
    bool OwnershipChecker::implementsCopy(Type* type) const {
        if (!type) return false;
        // Temel tipler genellikle Copy'dir.
        if (type->isIntType() || type->isFloatType() || type->isBoolType() || type->isCharType() || type->isReferenceType()) { // Referanslar da Copy'dir
             return true;
        }
        // String genellikle Copy değildir (slice olabilir?).
        if (type->isStringType()) return false; // Varsayım: String objesi veya slice Copy değil

        // Struct veya Enum tiplerinin Copy olup olmaması, tüm alanlarının/varyantlarının Copy olmasına ve Drop implementasyonunun olmamasına bağlıdır.
        if (type->isStructType()) {
            const StructType* structType = static_cast<const StructType*>(type);
            // Struct'ın Drop'ı varsa Copy olamaz
            if (hasDrop(const_cast<StructType*>(structType))) return false; // Drop kontrolü TypeSystem'de olmalı

            // Tüm alanları Copy ise Struct Copy'dir.
            for (const auto& field : structType->fields) {
                if (!implementsCopy(field.type)) return false;
            }
            return true; // Tüm alanlar Copy ve Drop yok
        }
         if (type->isEnumType()) {
            const EnumType* enumType = static_cast<const EnumType*>(type);
             // Enum'un Drop'ı varsa Copy olamaz
            if (hasDrop(const_cast<EnumType*>(enumType))) return false; // Drop kontrolü TypeSystem'de olmalı

            // Tüm varyantların ilişkili tipleri Copy ise Enum Copy'dir.
             for (const auto& variant : enumType->variants) {
                 if (variant.associatedType && !implementsCopy(variant.associatedType)) return false;
             }
             return true; // Tüm varyant tipleri Copy ve Drop yok
        }

        // Diğer tipler (Function, Array, Tuple) için Copy mantığını ekleyin.
        // Function pointerlar genellikle Copy'dir.
        // Array [T; size] eğer T Copy ise Copy'dir.
        // Tuple (T1, T2) eğer T1 ve T2 Copy ise Copy'dir.

        return false; // Varsayılan olarak Copy değil (Kaynak yönetimi varsa)
    }

    // Bir tipin Drop implementasyonu olup olmadığını kontrol et
    bool OwnershipChecker::hasDrop(Type* type) const {
         if (!type) return false;
        // String gibi bazı built-in tipler Drop'a sahip olabilir.
        if (type->isStringType()) return true; // Varsayım: String Drop'a sahip

        // Struct veya Enum tiplerinin Drop olup olmaması, herhangi bir alanının/varyantının Drop olmasına bağlıdır.
        if (type->isStructType()) {
            const StructType* structType = static_cast<const StructType*>(type);
            for (const auto& field : structType->fields) {
                if (hasDrop(field.type)) return true; // Herhangi bir alan Drop ise Struct Drop'tur
            }
            return false; // Hiçbir alan Drop değil
        }
         if (type->isEnumType()) {
            const EnumType* enumType = static_cast<const EnumType*>(type);
            for (const auto& variant : enumType->variants) {
                 if (variant.associatedType && hasDrop(variant.associatedType)) return true; // Herhangi bir varyantın tipi Drop ise Enum Drop'tur
            }
            return false; // Hiçbir varyant tipi Drop değil
        }

        // Pointerlar Drop değildir. Referanslar Drop değildir. Temel tipler (int, bool) Drop değildir.
        // Diğer tipler için Drop mantığını ekleyin.

        return false; // Varsayılan olarak Drop yok
    }


    // Bir sahiplik/ödünç alma hatası raporla
    void OwnershipChecker::reportOwnershipError(const TokenLocation& location, const std::string& message) {
        diagnostics.reportError(location, "Ownership Hatası: " + message);
    }

    // Ödünç alma çakışmasını raporla
    void OwnershipChecker::reportBorrowConflict(const TokenLocation& newBorrowLocation, bool isNewBorrowMutable, const SymbolInfo* borrowedSymbol, const ActiveBorrow& conflictingBorrow) {
        std::stringstream ss;
        ss << "Ödünç alma çakışması: '" << borrowedSymbol->name << "' değeri zaten ödünç alınmış.";
        ss << "\nYeni ödünç alma (" << (isNewBorrowMutable ? "&mut" : "&") << ") burada: " << newBorrowLocation.filename << ":" << newBorrowLocation.line << ":" << newBorrowLocation.column;
        ss << "\nÇakışan ödünç alma (" << (conflictingBorrow.isMutable ? "&mut" : "&") << ") burada başladı: " << conflictingBorrow.location.filename << ":" << conflictingBorrow.location.line << ":" << conflictingBorrow.location.column;
        // borrowScope bilgisi de eklenebilir
        reportOwnershipError(newBorrowLocation, ss.str());
    }

    // Belirli bir kapsam dışına çıkan tüm aktif ödünç almaları kaldır
    void OwnershipChecker::endBorrowsInScope(const Scope* scopeToEnd) {
        if (!scopeToEnd) return;

        // Tüm değişkenlerin aktif ödünç alma listelerini gez
        for (auto& pair : activeVariableBorrows) {
            // Listeyi, scopeToEnd'da veya daha iç kapsamda başlayan ödünç almaları içermeyecek şekilde filtrele
            auto& borrows = pair.second;
            borrows.erase(std::remove_if(borrows.begin(), borrows.end(),
                                         [scopeToEnd, &symbolTable = this->symbolTable](const ActiveBorrow& borrow) {
                                             // Ödünç almanın başladığı kapsam, bitirilen kapsamla aynı veya daha içteyse, bu ödünç alma biter.
                                             // Scope derinliklerini karşılaştırabiliriz.
                                              return borrow.borrowScope == scopeToEnd || borrow.borrowScope->depth > scopeToEnd->depth;

                                             // Daha güvenli: borrowScope'un scopeToEnd veya onun alt kapsamı olup olmadığını kontrol et.
                                             // Bu, scopeStack veya SymbolTable'ın parent zinciri üzerinden yapılabilir.
                                             // Basit implementasyon: Eğer borrowScope, scopeToEnd'in parent zincirinde yoksa veya kendisi değilse hala aktiftir.
                                             // borrowScope'un parent zincirinde scopeToEnd var mı?
                                             const Scope* current = borrow.borrowScope;
                                             while(current != nullptr) {
                                                 if (current == scopeToEnd) return true; // Ödünç alma bu kapsamda veya daha içte başladı, şimdi bitiyor.
                                                 current = current->parent;
                                             }
                                             return false; // Ödünç alma daha dış bir kapsamda başladı, hala aktif.
                                         }),
                          borrows.end());
        }
        // Eğer bir değişkenin hiç aktif ödünç alması kalmazsa, activeVariableBorrows'dan o girişi silebilirsiniz.
    }


    // Bir değişkenin mevcut durumunun belirli bir kullanım türüne izin verip vermediğini kontrol et
    bool OwnershipChecker::canUseVariable(const SymbolInfo* symbol, UseKind useKind, const TokenLocation& location) const {
        if (!symbol) return false; // Geçersiz sembol

        auto status_it = variableStatuses.find(symbol);
        if (status_it == variableStatuses.end()) {
            // Durumu takip edilmiyor? Bu bir hata olabilir (tanımlanmamış değişken kullanımı SEMA yakalar).
            // Veya sadece Copy olan bir geçici değer olabilir (handleExpressionResult yönetir).
            // Eğer symbol bir değişken sembolü ise ve durumu yoksa, bu bir internal hatadır.
             if (symbol->isVariable() && symbol->declarationNode) { // Orijinal tanımlanmış değişken
                 // diagnostics.reportInternalError(location, "Değişken '" + symbol->name + "' için durum takip edilmiyor.");
                 return false;
             }
             // Copy olan geçici değerler her zaman Move/Read/BorrowImmutable/BorrowMutable olabilir. Write olamaz.
             // Bu durumda durumu takip etmeye gerek yok, Copy kuralına bakılır.
             if (useKind == UseKind::Write) {
                 // diagnostics.reportError(location, "Geçici değer değiştirilemez."); // SEMA Expression analizi sırasında yakalar
                 return false; // Geçici değerlere yazılamaz
             }
             // Copy olan geçici değer diğer kullanımlara (Read, Move, Borrow) izin verir.
             return implementsCopy(symbol->type); // Geçici değer Copy ise diğer kullanımlar OK
        }

        VariableStatus currentStatus = status_it->second;

        // Duruma ve kullanıma göre kuralları kontrol et
        switch (currentStatus) {
            case VariableStatus::Owned:
                // Owned değer okunabilir, ödünç alınabilir.
                if (useKind == UseKind::Read || useKind == UseKind::BorrowImmutable || useKind == UseKind::BorrowMutable) return true;
                // Owned değer atanabilir (yazılabilir) mi? Değişkenin kendisi mutable olmalı.
                if (useKind == UseKind::Write) return symbol->isMutable;
                // Owned değer taşınabilir mi? Her zaman (eğer daha önce taşınmadıysa ve Drop edilmediyse).
                if (useKind == UseKind::Move) return true; // Durumu Moved olarak güncellenecek.
                break; // Diğer durumlar (Dropped, Moved gibi) burada Owned olarak görünmemeli
            case VariableStatus::Moved:
                 // Taşınmış değer kullanılamaz (re-assignment hariç).
                 if (useKind != UseKind::Write) { // Yazma (re-assignment) dışında
                     reportOwnershipError(location, "'" + symbol->name + "' değeri daha önce taşındı ve kullanılamaz.");
                     return false;
                 }
                 // Moved değerin üzerine yazılabilir (yeniden sahiplenme).
                 return true; // Kullanım (yazma) geçerli, durum Owned olarak güncellenecek.
            case VariableStatus::BorrowedImmutable:
                // Immutable ödünç alınmışken: Sadece okunabilir veya yeni immutable referans alınabilir.
                 if (useKind == UseKind::Read || useKind == UseKind::BorrowImmutable) return true;
                // Yazma, taşıma veya mutable ödünç alma HATA.
                reportOwnershipError(location, "'" + symbol->name + "' değeri şu anda immutable ödünç alınmış ve " + getUseKindString(useKind) + " yapılamaz.");
                return false;
            case VariableStatus::BorrowedMutable:
                 // Mutable ödünç alınmışken: Kendi kullanımı hariç başka kullanım HATA.
                 // Hangi kullanımın "kendi kullanımı" olduğunu belirlemek karmaşık olabilir (dereference üzerinden erişim gibi).
                 // Şimdilik, Mutable ödünç alınmışken herhangi bir doğrudan kullanımın (Read, Write, Move, Borrow) HATA olduğunu varsayalım.
                 // Dereference (*) operatörü, checkUsageRules'ı ödünç alınan değer üzerinde çağırmalı.
                 reportOwnershipError(location, "'" + symbol->name + "' değeri şu anda mutable ödünç alınmış ve " + getUseKindString(useKind) + " yapılamaz.");
                 return false;
            case VariableStatus::Dropped:
                 // Drop edilmiş değer kullanılamaz.
                 reportOwnershipError(location, "'" + symbol->name + "' değeri zaten kapsamdan çıktı ve kullanılamaz.");
                 return false;
            case VariableStatus::Uninitialized:
                 // Değer atanmadan kullanılamaz (atama hariç).
                 if (useKind != UseKind::Write && useKind != UseKind::Move) { // Yazma veya taşıma (ilk atama) hariç
                      reportOwnershipError(location, "'" + symbol->name + "' değişkeni henüz başlatılmadı.");
                      return false;
                 }
                 return true; // İlk atama geçerli, durum Owned olarak güncellenecek.
            // PartiallyMoved durumu için mantık ekleyin.
        }

        return false; // Varsayılan olarak izin verme (ulaşılmaması gereken durum)
    }

    // Bir ifade sonucunun (geçici değer) sahiplik/copy/drop durumunu yönet.
    void OwnershipChecker::handleExpressionResult(ExpressionAST* expr) {
        if (!expr || !expr->resolvedSemanticType) return;

        Type* resultType = expr->resolvedSemanticType;
        // Eğer ifade sonucu kopyalanamayan (non-Copy) bir tip ise, bu bir r-value taşıma işlemidir.
        // Bu geçici değer, bir değişkene atanmazsa veya bir fonksiyona taşınarak geçirilmezse,
        // ifadenin sonunda Drop edilmesi gerekebilir.
        // Bu karmaşık bir konu, genellikle CodeGen'de IR seviyesinde veya AST gezinmesi sırasında
        // belirli düğüm türleri (ExpressionStatement, If/Match dalları sonu) için Drop çağrıları işaretlenir.

        // Basit implementasyon: Eğer non-Copy bir geçici değer, bir değişkene taşınmıyorsa
        // veya bir fonksiyona argüman olarak taşınarak verilmiyorsa (handleAssignment, handleFunctionCall içinde ele alınır),
        // ifadenin bittiği yerde (örn: ExpressionStatement) bu değer Drop edilmelidir.
        // Bu metod, Drop edilmesi gereken geçici değerleri izlemek için kullanılabilir.
        if (!implementsCopy(resultType) && hasDrop(resultType)) {
            // Bu geçici değerin Drop edilmesi gerekebilir.
            // Nerede drop edileceğini AST yapısına göre belirlemek gerekir.
            // Örneğin, eğer bu ifade bir deyim ifadesinin (ExpressionStatement) köküyse, Drop edilir.
            // Eğer başka bir ifadenin parçasıysa (a + b'deki a veya b), Drop edilmez, bir sonraki adıma taşınır.
        }
    }


    // =======================================================================
    // Semantic Analyzer tarafından Çağrılan Metodların Implementasyonu
    // =======================================================================

    // Yeni bir kapsam açılırken çağrılır
    void OwnershipChecker::enterScope(const Scope* scope) {
        assert(scope != nullptr && "Cannot enter a null scope.");
        scopeStack.push_back(scope);
        // Kapsamla ilgili ek durumları başlatabilirsiniz.
         std::cout << "DEBUG: OC - Entered Scope Depth " << scope->depth << std::endl;
    }

    // Mevcut kapsam kapanırken çağrılır (Kaynakların drop edildiği yer)
    void OwnershipChecker::exitScope() {
        assert(!scopeStack.empty() && "Cannot exit scope when scope stack is empty.");

        const Scope* scopeToExit = scopeStack.back();
         std::cout << "DEBUG: OC - Exiting Scope Depth " << scopeToExit->depth << std::endl;

        // Bu kapsamda biten ödünç almaları kaldır (endBorrowsInScope helper kullanır)
        endBorrowsInScope(scopeToExit);

        // Bu kapsamda tanımlanmış değişkenleri işle (Drop, Moved kontrolü)
        // SymbolTable::getCurrentScope() metodunu kullanabiliriz veya exitScope'a Scope* parametresi geçirebiliriz.
        // SymbolTable'ın metodunu kullanalım.
        // exitScope çağrıldığında SymbolTable'da scope henüz çıkmamış olabilir, veya çıkmış olabilir.
        // SymbolTable::exitScope'un önce çağrıldığını varsayalım.
        // Bu durumda SymbolTable::getCurrentScope() üst kapsamı döner.
        // Kapanan kapsamdaki değişkenlere erişmek için SymbolTable'da özel bir metod veya exitScope'a değişken listesi geçirmek gerekebilir.
        // Varsayalım SymbolTable, exitScope'tan önce kapanan kapsamdaki değişkenleri bize verebilir.

        // Örneğin:
         for (const auto& pair : symbolTable.getVariablesInScope(scopeToExit)) {
             const SymbolInfo* varSymbol = pair.second.get(); // shared_ptr'dan ham pointer
             auto status_it = variableStatuses.find(varSymbol);
             if (status_it != variableStatuses.end()) {
                 VariableStatus finalStatus = status_it->second;
                 if (finalStatus == VariableStatus::Owned) {
        //             // Değer hala sahip olunan durumda kapsamdan çıktı. Drop edilmesi gerekiyor mu?
                     if (hasDrop(varSymbol->type)) {
        //                 // Mark for drop (CodeGen'e bilgi veya error eğer Drop olamayacak durumdaysa)
                           diagnostics.reportInfo(varSymbol->location, "'" + varSymbol->name + "' kapsamdan çıkarken Drop ediliyor.");
                     } else {
        //                 // Copy olan veya Drop olmayan tipler Drop edilmez.
                     }
                 } else if (finalStatus == VariableStatus::BorrowedImmutable || finalStatus == VariableStatus::BorrowedMutable) {
        //              // Kapsam dışına çıkan bir variable hala ödünç alınmış durumda olmamalı.
        //              // endBorrowsInScope bu borrow'ları kaldırdı, ama eğer variable'ın durumu 'Borrowed' ise bu bir hata olabilir.
        //              // Hayır, variable'ın durumu, o variable'a dışarıdan borç alınıp alınmadığını gösterir.
        //              // Variable'ın durumu sadece Owned veya Moved olarak kalmalıdır kapsam çıkışında.
        //              // Eğer durumu Borrowed ise, bu variable'dan yapılan bir borrow hala yaşıyor demektir, bu da bir hata (lifetimelar uymuyor).
                      if (finalStatus != VariableStatus::Moved && finalStatus != VariableStatus::Dropped && finalStatus != VariableStatus::Owned) {
                          reportOwnershipError(varSymbol->location, "'" + varSymbol->name + "' değişkeni kapsamdan çıkarken geçerli olmayan bir durumda (" + getVariableStatusString(finalStatus) + ").");
                      }
                 }
        //          // Moved veya Dropped olanlar zaten işlenmiş.
             }
        //      // VariableStatus map'inden bu değişkene ait girişi kaldırın.
              variableStatuses.erase(varSymbol);
         }


        scopeStack.pop_back(); // Kapsamı yığından çıkar
    }

    // Bir değişken bildirimi analiz edilirken çağrılır
    void OwnershipChecker::recordVariableDeclaration(const SymbolInfo* symbol, const TokenLocation& location) {
        assert(symbol != nullptr && symbol->isVariable() && "Invalid symbol for variable declaration.");
        assert(scopeStack.back() == symbolTable.getCurrentScope() && "Scope mismatch between OC and ST."); // Scope'ların senkronize olduğu varsayım

        // Değişkenin başlangıç durumunu belirle.
        // Eğer başlangıç değeri yoksa, Uninitialized. Varsa, Owned (copy veya move'a bağlı).
        // Şimdilik basitçe Owned olarak başlatalım, ilk atama/initializer analyzeAssignment/handleVariableUse'da yönetilir.
        variableStatuses[symbol] = VariableStatus::Owned;
         std::cout << "DEBUG: OC - Declared Variable '" << symbol->name << "' at Depth " << scopeStack.back()->depth << std::endl;
    }


    // Bir değişkenin kullanımı analiz edilirken çağrılır
    void OwnershipChecker::handleVariableUse(const SymbolInfo* symbol, UseKind useKind, const TokenLocation& location, const ASTNode* useNode) {
        assert(symbol != nullptr && symbol->isVariable() && "Invalid symbol for variable use.");
         std::cout << "DEBUG: OC - Using Variable '" << symbol->name << "' (" << getUseKindString(useKind) << ") at Depth " << scopeStack.back()->depth << std::endl;

        // Değişkenin bu kullanıma izin verip vermediğini kontrol et
        if (!canUseVariable(symbol, useKind, location)) {
            // Hata zaten canUseVariable içinde raporlandı.
            // Durum güncellemeyi atla.
            return;
        }

        // Kullanım türüne göre değişkenin durumunu güncelle
        VariableStatus currentStatus = variableStatuses.at(symbol); // CanUseVariable'dan geçtiyse var demektir.

        switch (useKind) {
            case UseKind::Read:
                 // Durum değişmez (Owned, BorrowedImmutable)
                 break;
            case UseKind::Write:
                 // Durum Owned'a döner (eğer Moved veya Uninitialized idiyse) veya Owned kalır.
                 // Eğer Owned iken yazılıyorsa, isMutable kontrolü canUseVariable'da yapıldı.
                 variableStatuses[symbol] = VariableStatus::Owned; // Üzerine yazıldığı için yeni değere sahip olur
                 // Yazma işlemi var olan borçlarla çakışabilir. canUseVariable'da Borrowed kontrolü yapıldı.
                 break;
            case UseKind::Move:
                // Eğer tipi Copy değilse, durum Moved olur.
                if (!implementsCopy(symbol->type)) {
                    variableStatuses[symbol] = VariableStatus::Moved;
                     // Bu değişkenle ilişkili tüm aktif borçlar geçersiz hale gelir.
                     // Bu borçları activeVariableBorrows map'inden silmeniz veya geçersiz olarak işaretlemeniz gerekir.
                      activeVariableBorrows.erase(symbol); // Basit yöntem: Tümü bitti
                     // Daha doğru: Her ActiveBorrow'un valid flag'i olabilir.
                     // Borç veren ifadenin (UnaryOpAST) de referansı geçersiz hale gelir.
                }
                // Eğer tipi Copy ise, durum Owned kalır (değer kopyalanır).
                break;
            case UseKind::BorrowImmutable:
                // Yeni immutable borrow eklenir. checkBorrowRules kontrolü canUseVariable'da yapıldı.
                // Buraya ActiveBorrow objesi eklenmelidir.
                 const Scope* borrowScope = ... // Borcun kapsamı (genellikle mevcut veya daha dış)
                 activeVariableBorrows[symbol].emplace_back(useNode, location, false, borrowScope, symbol); // useNode -& veya &mut AST'si
                break;
            case UseKind::BorrowMutable:
                 // Yeni mutable borrow eklenir. checkBorrowRules kontrolü canUseVariable'da yapıldı.
                  const Scope* borrowScope = ... // Borcun kapsamı
                  activeVariableBorrows[symbol].emplace_back(useNode, location, true, borrowScope, symbol); // useNode -&mut AST'si
                break;
        }
        // Eğer durum BorrowedMutable veya BorrowedImmutable olarak değiştiyse (Borrow use kind),
         canUseVariable'da checkBorrowRules'ı çağırdık. Oraya bak.
    }

    // Atama ifadesi analiz edilirken çağrılır
    void OwnershipChecker::handleAssignment(const ExpressionAST* leftExpr, const ExpressionAST* rightExpr, const TokenLocation& location) {
        // SEMA analyzeAssignment içinde expressionları analiz etti.
        // Sağ tarafın tipi rightExpr->resolvedSemanticType.
        // Sol taraf bir l-value olmalı (SEMA check eder) ve bir değişken, alan veya index erişimi olabilir.
        // Sol tarafın kaynağı (SymbolInfo, Alan referansı, Index referansı) belirlenmeli.
        // Sahiplik/Ödünç alma, sağ taraftaki değerden sol tarafa geçer.

        // Örneğin, eğer sol taraf basit bir değişken ise:
        if (const IdentifierAST* leftId = dynamic_cast<const IdentifierAST*>(leftExpr)) {
            if (leftId->resolvedSymbol && leftId->resolvedSymbol->isVariable()) {
                const SymbolInfo* targetSymbol = leftId->resolvedSymbol;
                Type* valueType = rightExpr->resolvedSemanticType;

                // Eğer atanacak değer (sağ taraf) bir değişkense ve tipi Copy değilse, bu bir Move'dur.
                // Eğer sağ taraf geçici bir değerse (literal, fonksiyon çağrı sonucu vb.) ve tipi Copy değilse, bu da bir Move'dur.
                // Eğer sağ taraf bir değişkense ve tipi Copy ise, bu bir Copy'dir (handleVariableUse::Move içinde Copy mantığı var).
                // Move mi Copy mi? Sağ tarafın kaynağına bağlı.
                // Eğer sağ taraf bir IdentifierAST ise ve Copy değilse -> sağ taraf taşınıyor.
                 if (const IdentifierAST* rightId = dynamic_cast<const IdentifierAST*>(rightExpr)) {
                     if (rightId->resolvedSymbol && rightId->resolvedSymbol->isVariable() && !implementsCopy(valueType)) {
                //          // Sağdaki değişken taşınıyor. O değişkenin durumunu Moved yap.
                         handleVariableUse(rightId->resolvedSymbol, UseKind::Move, rightId->location, rightId);
                     }
                 }
                // Eğer sağ taraf geçici değer ve non-Copy ise -> değer move ediliyor.
                 handleExpressionResult bu geçici değeri Drop olarak işaretleyebilir,
                // ama burada atandığı için Drop edilmez, targetSymbol'a taşınır.
                 if (!implementsCopy(valueType)) {
                //    // Sağdaki non-Copy değer taşınıyor.
                 }


                // Sol taraftaki değişkenin durumunu güncelle.
                // Eğer Copy ise durum Owned kalır. Non-Copy ise, sağdan gelen değer artık bu değişkene aittir.
                // Eğer hedef değişken daha önce bir değere sahipse (Owned), o değer Drop edilmelidir.
                // Bu atama işlemi sırasında OwnershipChecker'ın yapması gerekenler:
                // 1. Sağ taraftaki değerin kaynağının (variable, borrow, temporary) bu kullanıma (move/copy) izin verdiğini kontrol et.
                // 2. Eğer sağdaki değer taşınıyorsa (non-Copy ve l-value değil veya l-value ama taşınıyor), sağdaki kaynağın durumunu güncelle (Moved).
                // 3. Eğer sol taraftaki değişken daha önce bir değere sahipse (Owned) ve o değer Drop gerektiriyorsa, o değeri Drop olarak işaretle.
                // 4. Sol taraftaki değişkenin durumunu Owned olarak ayarla.

                // Basit İskelet: Sadece sol değişkenin üzerine yazıldığını belirten kullanım sinyali gönder.
                handleVariableUse(targetSymbol, UseKind::Write, location, leftId); // isMutable kontrolü canUseVariable'da yapıldı.
                // Sağdaki değerin taşınması handleVariableUse içinde UseKind::Move ile yönetilir.
            } else {
                // Sol taraf Identifier ama Variable SymbolKind değil (fn, struct, enum gibi) veya resolvedSymbol yok.
                // Bu SEMA hatasıdır (atanabilir olmalıydı).
            }
        }
        // Sol taraf MemberAccessAST veya IndexAccessAST ise daha karmaşık mantık gerekir.
        // Alanlara veya dizi elemanlarına atama, kısmi sahiplik/ödünç alma gerektirebilir.
        // handleMemberAccessAssignment, handleIndexAssignment gibi özel metodlar gerekebilir.
         ownershipChecker.handleAssignment(leftExpr, rightExpr, location); // Genel handler
    }

    // Fonksiyon çağrısı analiz edilirken çağrılır
    void OwnershipChecker::handleFunctionCall(const CallExpressionAST* callExpr) {
        // SEMA analyzeCallExpression içinde her şeyi analiz etti.
        // Çağrılan fonksiyonun SymbolInfo'su: callExpr->resolvedCalleeSymbol
        // Argüman listesi: callExpr->arguments
        // Dönüş tipi: callExpr->resolvedSemanticType

        // Argümanları işle:
        // Her argümanın nasıl geçtiğini belirle (by value/move, &T, &mut T). Fonksiyon imzasındaki parametre tipine bakılır.
        // Örneğin, parametre T ise ve argüman değişken `x` ise:
        // - Eğer tipi Copy ise: x okunur (UseKind::Read), değeri kopyalanır, kopya fonskiyona taşınır (Move). x'in durumu değişmez.
        // - Eğer tipi Non-Copy ise: x taşınır (UseKind::Move), OwnershipChecker x'in durumunu Moved yapar. x fonksiyona taşınır.
        // Örneğin, parametre &T ise ve argüman değişken `x` ise:
        // - Immutable ödünç alma yapılır (&x). handleVariableUse(x, UseKind::BorrowImmutable, ...) çağrılır.
        // Örneğin, parametre &mut T ise ve argüman değişken `x` ise:
        // - Mutable ödünç alma yapılır (&mut x). handleVariableUse(x, UseKind::BorrowMutable, ...) çağrılır.

        // handleVariableUse metodunu uygun UseKind ile her argüman için çağırın.
        // Örneğin:
         for (size_t i = 0; i < callExpr->arguments.size(); ++i) {
             const ExpressionAST* argExpr = callExpr->arguments[i].get();
             Type* paramType = ... ; // Fonksiyon imzasından parametre tipi
             UseKind argUseKind = ... ; // Parametre tipine göre (T -> Move, &T -> BorrowImmutable, &mut T -> BorrowMutable)

        //     // Argüman bir değişken mi?
             if (const IdentifierAST* argId = dynamic_cast<const IdentifierAST*>(argExpr)) {
                  if (argId->resolvedSymbol && argId->resolvedSymbol->isVariable()) {
                      handleVariableUse(argId->resolvedSymbol, argUseKind, argId->location, argId);
                  } else {
        //             // Argüman değişken değil (örn: literal, fonksiyon çağrısı). Geçici değer.
        //             // Geçici değerlerin sahiplik durumu karmaşıktır. handleExpressionResult yönetir.
                     // Eğer argUseKind Move ise ve geçici non-Copy ise, o geçici değer buraya taşınıyor ve Drop edilmeyecek.
                  }
             } else {
        //         // Argüman karmaşık ifade (binary op, call, member access). Sonucu geçici değerdir.
        //         // Benzer şekilde geçici değer mantığı.
             }
         }

        // Dönüş değerini işle:
        // Fonksiyonun dönüş tipi (callExpr->resolvedSemanticType). Eğer non-Copy ise, bu bir r-value Move'dur.
        // Çağrı ifadesinin sonucunu alan yere göre (bir değişkene atanıyor mu, başka fonksiyona mı taşınıyor)
        // handleExpressionResult veya handleAssignment içinde yönetilecektir.
         ownershipChecker.handleFunctionCall(callExpr); // Genel handler
    }

    // Referans oluşturma (&expr veya &mut expr) analiz edilirken çağrılır
    void OwnershipChecker::handleReferenceCreation(const UnaryOpAST* unaryOp, const SymbolInfo* borrowedSymbol) {
        // SEMA analyzeUnaryOp içinde operandı analiz etti ve tipi aldı.
        // Operandın bir l-value olması ve mutable referans alınıyorsa mutable olması SEMA tarafından kontrol edildi.
        // borrowedSymbol, ödünç alınan değişkenin SymbolInfo'suna pointer (eğer operand değişken ise).
        // Eğer operand bir alan veya dizi elemanı ise, borrowedSymbol null olabilir ve TargetNode kullanılabilir.

        assert(unaryOp != nullptr && (unaryOp->op == Token::TOK_AND || unaryOp->op == Token::TOK_MUT) && "Invalid node for reference creation.");

        bool isMutableBorrow = (unaryOp->op == Token::TOK_MUT);
        // const ASTNode* borrowedTargetNode = unaryOp->operand.get(); // Ödünç alınan ifadenin AST düğümü

        // Ödünç alınan değerin SymbolInfo'sunu veya kaynağını bul.
        // Eğer operand Identifier ise, SymbolInfo'su borrowedSymbol'dur.
        // Eğer operand MemberAccess veya IndexAccess ise, SEMA resolvedMemberSymbol'ü set etmiş olmalı.
        // const SymbolInfo* targetSymbolForBorrow = borrowedSymbol; // Basit değişken ödünç alma
        // Karmaşık durumlar için hedefi belirleme mantığı ekleyin.

        // Ödünç alma kurallarını kontrol et (checkBorrowRules helper kullanır)
        if (targetSymbolForBorrow) { // Sadece değişken ödünç almayı handle edelim şimdilik
             checkBorrowRules(targetSymbolForBorrow, isMutableBorrow, unaryOp->location, scopeStack.back(), const_cast<UnaryOpAST*>(unaryOp));
             // CheckBorrowRules başarılıysa, ActiveBorrow nesnesini activeVariableBorrows map'ine ekler.
        } else {
             // Karmaşık ifadelerden (geçici değerler, fonksiyon çağrı sonucu vb.) ödünç alma kuralları.
             // Genellikle geçici değerlerden mutable referans almak yasaktır. Immutable referanslar mümkündür.
              ownershipChecker.checkBorrowOnTemporary(...);
        }
    }

    // Dereference (*expr) analiz edilirken çağrılır
    void OwnershipChecker::handleDereference(const UnaryOpAST* unaryOp) {
        assert(unaryOp != nullptr && unaryOp->op == Token::TOK_STAR && "Invalid node for dereference.");
        // SEMA analyzeUnaryOp içinde operandı analiz etti ve tipinin referans/pointer olduğunu doğruladı.
        // Operandın tipi: unaryOp->operand->resolvedSemanticType (&T veya &mut T)

        // Dereference, ödünç alınan değere erişimdir.
        // Bu erişim, ödünç alma kurallarına (mutable borrow aktifken sadece mutable referans sahibi erişebilir) uymalıdır.
        // Ayrıca, referansın işaret ettiği değerin (borrowedSymbol veya target) hala geçerli (Moved veya Dropped değil) olduğunu kontrol etmelidir.
        // Referansın yaşam süresinin (borrowScope) hala aktif olduğunu da kontrol etmelidir.

        // Ödünç alınan değerin SymbolInfo'su veya kaynağını belirleyin.
        // Referansın kendisi (operand) bir değişkense (let ref = &x; *ref), ref'in resolvedSymbol'ü kullanılır.
        // Eğer referans geçici ise (&x; *&x), x'in resolvedSymbol'ü veya kaynağı bulunur.
         const SymbolInfo* borrowedSymbol = ... // Ödünç alınan kaynağı belirleyin

        // Ödünç alınan değere erişim türünü belirleyin (okuma/yazma). Bu, *expr'nin kullanıldığı bağlama bağlıdır.
        // Örneğin, `*ptr = 5;` yazmadır. `y = *ptr;` okumadır.
        // Bu bilgiyi handleDereference'a parametre olarak geçirmek gerekebilir (UseKind).
        // SEMA analyzeAssignment içinde sol tarafın l-value olup olmadığını ve write kullanımını belirler.
         ownershipChecker.checkUsageRules(borrowedSymbol, actualUseKind, unaryOp->location); // Ödünç alınan değerin kullanımını kontrol et

        // Yaşam süresi kontrolü: Kullanılan referansın (unaryOp->operand) geçerli bir ödünç almadan geldiğini doğrula.
        // Orijinal ödünç almayı activeVariableBorrows listesinde bulup scope'unun hala geçerli olup olmadığını kontrol et.
         std::cout << "DEBUG: OC - Dereferencing at " << unaryOp->location.filename << ":" << unaryOp->location.line << std::endl; // Debug
    }

    // Return deyimi analiz edilirken çağrılır
    void OwnershipChecker::handleReturn(const ReturnStatementAST* returnStmt) {
        // Return deyimi, içinde bulunduğu fonksiyonun kapsamından çıkılmasına neden olur.
        // Kapsam dışına çıkan tüm yerel değişkenler (returnStmt'nin scope'unda tanımlı olanlar) Drop edilmelidir (eğer sahiplerse ve taşınmadılarsa).
        // Ödünç almalar da bu kapsamda biter.
        // Dönüş değeri, fonksiyonun çağrı noktasına taşınır. Eğer dönüş tipi non-Copy ise bu bir Move'dur.

        // Kapsam çıkışını yönet (exitScope çağrısı gibi, ama return özel bir çıkış)
        // Tüm iç içe geçmiş kapsamları (return deyiminin olduğu kapsamdan fonksiyonun kapsamına kadar) teker teker çıkış yapar gibi yönetin.
        // SymbolTable'ı kullanarak return deyiminin ait olduğu kapsamı ve üstlerini belirleyin.
        // SymbolTable::exitScope'u çağırmak yerine, özel bir return-exit mantığı gerekebilir.

        // Örneğin: returnStmt'nin ait olduğu kapsamı bulun.
         const Scope* returnScope = symbolTable.getScopeForNode(returnStmt); // SymbolTable'da böyle bir helper olabilir.
         while (scopeStack.back() != returnScope && !scopeStack.empty()) {
             exitScope(); // Return deyimine kadar olan ara kapsamları normal çıkış gibi işle (Drop, Borrow sonu)
         }
         assert(scopeStack.back() == returnScope && "Return scope mismatch."); // Return deyiminin kapsamına ulaştık.
         exitScope(); // Return deyiminin kendi kapsamından çık.

        // Dönüş değerinin sahipliğini işle
        // returnStmt->returnValue ifadesinin sonucunu (geçici değer veya değişken değeri) fonksiyondan dışarı taşıma.
        // Eğer returnValue bir değişken ise ve tipi non-Copy ise, o değişkenin durumu Moved olur.
        // Eğer returnValue geçici bir değer ise ve tipi non-Copy ise, Drop edilmeden dışarı taşınır.
         ownershipChecker.handleReturn(returnStmt); // Genel handler
    }

    // Break veya Continue deyimi analiz edilirken çağrılır
    void OwnershipChecker::handleJump(const StatementAST* jumpStmt) {
        // Break veya Continue, içinde bulunduğu döngünün (ve varsa aradaki diğer scopeların) sonuna atlamaya neden olur.
        // Atlanan scopelarda tanımlı olan değişkenler Drop edilmelidir.
        // Atlanan scopelarda başlayan ödünç almalar sona ermelidir.

        // jumpStmt'nin ait olduğu kapsamı bulun.
        // jumpStmt'nin hedef döngüsünün kapsamını bulun (SEMA break/continue analizinde bu bilgiyi tutar).
        // Atlanan tüm ara kapsamları exitScope gibi işlemeniz gerekir.
         ownershipChecker.handleJump(jumpStmt); // Genel handler
    }

    // Match kollarının veya If/Else dallarının birleştiği yerde çağrılabilir
    void OwnershipChecker::handleBranchMerge(const TokenLocation& mergeLocation) {
        // Dallanmış yolların (Match kolları, If/Else dalları) birleştiği yerde,
        // dallar içinde değişen değişken durumlarının (Owned -> Moved, Borrowed, vb.) birleştirilmesi gerekir.
        // Bir değişken bir yolda taşınmış, diğer yolda taşınmamışsa, birleşme noktasında o değişken kullanılamaz hale gelebilir.
        // Aktif ödünç almalar da birleştirilmelidir.
        // Bu çok karmaşık bir analizdir ve kontrol akışı grafiği (CFG) veya benzeri bir yapı gerektirebilir.
        // Basit bir implementasyon: Eğer bir değişkenin durumu dallar arasında farklılık gösteriyorsa (Owned vs Moved),
        // birleşme noktasında o değişkeni Moved olarak işaretle. Aktif ödünç almaları birleştir (tüm dallardaki aktif borçlar birleşme noktasında hala aktifse).
         ownershipChecker.handleBranchMerge(mergeLocation); // Genel handler
    }


    // Internal Helper Method Implementations (checkBorrowRules, canUseVariable vb.)
    // Bu metodların implementasyonları, yukarıdaki açıklamalara ve CNT'nin kesin kurallarına göre detaylandırılmalıdır.

    // Örnek: checkBorrowRules implementasyon iskeleti
     bool OwnershipChecker::checkBorrowRules(const SymbolInfo* borrowedSymbol, bool isNewBorrowMutable, const TokenLocation& borrowLocation, const Scope* borrowScope, const ASTNode* sourceNode) {
         assert(borrowedSymbol != nullptr && borrowedSymbol->isVariable() && "Can only borrow variables directly."); // Basitlik varsayımı

         // Değişkenin anlık durumuna bak
         VariableStatus currentStatus = variableStatuses.at(borrowedSymbol); // recordVariableDeclaration ile eklendiğini varsayalım

         // Eğer değişken Moved veya Dropped ise ödünç alınamaz
         if (currentStatus == VariableStatus::Moved || currentStatus == VariableStatus::Dropped) {
             reportOwnershipError(borrowLocation, "'" + borrowedSymbol->name + "' değeri " + getVariableStatusString(currentStatus) + " durumda, ödünç alınamaz.");
             return false;
         }

         // Eğer yeni ödünç alma mutable ise (&mut)
         if (isNewBorrowMutable) {
             // Değişken mutable olmalı
             if (!borrowedSymbol->isMutable) {
                 reportOwnershipError(borrowLocation, "'" + borrowedSymbol->name + "' değişkeni mutable değil, mutable referans (&mut) alınamaz.");
                 return false;
             }
             // Bu değişkende şu anda *başka* hiçbir aktif ödünç alma OLMAMALI (& veya &mut)
             // Değişkenin kendisi şu anda ödünç alınmış durumda OLMAMALI (status != Owned)
             if (currentStatus != VariableStatus::Owned) {
                 reportOwnershipError(borrowLocation, "'" + borrowedSymbol->name + "' değeri şu anda " + getVariableStatusString(currentStatus) + " durumda, mutable referans (&mut) alınamaz.");
                 return false;
             }
             // Bu değişkene ait aktif borç listesi boş olmalı
             auto borrow_list_it = activeVariableBorrows.find(borrowedSymbol);
             if (borrow_list_it != activeVariableBorrows.end() && !borrow_list_it->second.empty()) {
                 // Çakışma var! İlk çakışan ödünç almayı bulup raporla.
                 const ActiveBorrow& conflictingBorrow = borrow_list_it->second.front(); // İlk çakışan (herhangi biri)
                 reportBorrowConflict(borrowLocation, true, borrowedSymbol, conflictingBorrow);
                 return false;
             }
         }
         // Eğer yeni ödünç alma immutable ise (&)
         else {
             // Bu değişkende şu anda *aktif* bir *mutable* ödünç alma OLMAMALI (&mut)
              auto borrow_list_it = activeVariableBorrows.find(borrowedSymbol);
              if (borrow_list_it != activeVariableBorrows.end()) {
                  for(const auto& active : borrow_list_it->second) {
                      if (active.isMutable) {
                          // Çakışma var! Aktif mutable ödünç alma var.
                          reportBorrowConflict(borrowLocation, false, borrowedSymbol, active);
                          return false;
                      }
                  }
              }
         }

         // Kontrollerden geçtiyse, yeni ödünç almayı aktif listeye ekle.
         activeVariableBorrows[borrowedSymbol].emplace_back(sourceNode, borrowLocation, isNewBorrowMutable, borrowScope, borrowedSymbol);
         // Değişkenin durumunu güncelle (BorrowedImmutable veya BorrowedMutable)
         variableStatuses[borrowedSymbol] = isNewBorrowMutable ? VariableStatus::BorrowedMutable : VariableStatus::BorrowedImmutable;

         return true; // Ödünç alma başarılı
     }

     // Örnek: canUseVariable implementasyon iskeleti (yukarıdaki versiyon daha tam)
      bool OwnershipChecker::canUseVariable(const SymbolInfo* symbol, UseKind useKind, const TokenLocation& location) const { ... }


} // namespace cnt_compiler
