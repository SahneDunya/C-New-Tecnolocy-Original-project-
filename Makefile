# Derleyici ve Bayraklar
CC = clang
CFLAGS = -Wall -Wextra -std=c11 -I./include # include dizinini ekle

# Bağımlılıklar
MUSL_INCLUDE = /path/to/musl-1.2.5/include # Musl başlık dosyalarının yolu
CONAN_INCLUDE = /path/to/conan-develop2/include # Conan başlık dosyalarının yolu
LDFLAGS = -L/path/to/musl-1.2.5/lib -L/path/to/conan-develop2/lib -lmusl -lconan # Musl ve Conan kütüphanelerinin yolu ve bağlantı bayrakları

# Kaynak Dosyaları
SRCS =

# Nesne Dosyaları
OBJS = $(SRCS:.c=.o)

# Hedef
TARGET = cnt_tools

# Başlık Dosyaları
HEADERS =

# Varsayılan Hedef
all: $(TARGET)

# Hedef Oluşturma
$(TARGET): $(OBJS)
 $(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(OBJS)

# Nesne Dosyası Oluşturma
%.o: %.c $(HEADERS)
 $(CC) $(CFLAGS) -c $< -o $@

# Temizleme
clean:
 rm -f $(OBJS) $(TARGET)

# Yeniden Oluşturma
rebuild: clean all