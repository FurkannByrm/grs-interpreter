# GRS Interpreter

GRS/KRL Robot Programming Language Interpreter

## 📖 Genel Bakış / Overview

GRS Interpreter, endüstriyel robot programlama dillerini (GRS/KRL) çalıştırabilen bir yorumlayıcıdır. Bu proje, robot programlarının analiz edilmesi, derlenmesi ve çalıştırılması için kapsamlı bir altyapı sağlar.

GRS Interpreter is an interpreter for industrial robot programming languages (GRS/KRL). This project provides a comprehensive infrastructure for analyzing, compiling, and executing robot programs.

## ✨ Özellikler / Features

- 🔤 **Lexical Analysis**: Token tabanlı kod analizi / Token-based code analysis
- 🌳 **AST Parser**: Abstract Syntax Tree oluşturma / Abstract Syntax Tree generation
- 🔄 **Instruction Generator**: Ara kod üretimi / Intermediate code generation
- ⚙️ **Executor**: Program çalıştırma motoru / Program execution engine
- 🎨 **IDE Support**: ZeroBrane Studio ve Scintilla desteği / ZeroBrane Studio and Scintilla support

## 🚀 Hızlı Başlangıç / Quick Start

### Gereksinimler / Requirements

- CMake 3.10 veya üzeri / CMake 3.10 or higher
- C++17 uyumlu derleyici / C++17 compatible compiler
- [constexpr_map](https://github.com/serge-sans-paille/frozen) kütüphanesi

### Derleme / Building

```bash
# Depo klonlama / Clone repository
git clone https://github.com/FurkannByrm/grs-interpreter.git
cd grs-interpreter/grs_interpreter

# Build klasörü oluşturma / Create build directory
mkdir build
cd build

# CMake ile yapılandırma / Configure with CMake
cmake ..

# Derleme / Build
make

# Çalıştırma / Run
./interpreter
```

## 💻 IDE Kurulumu / IDE Setup

GRS dili için tam IDE desteği mevcuttur! / Full IDE support is available for GRS language!

### Desteklenen IDE'ler / Supported IDEs

- 🌟 **ZeroBrane Studio** - Tam özellikli IDE desteği / Full-featured IDE support
  - Syntax highlighting
  - Autocomplete
  - Code folding
  - Integrated interpreter
  
- 📝 **Scintilla-based Editors**
  - Notepad++ (Windows)
  - SciTE (Cross-platform)
  - Geany (Linux)

### Kurulum / Installation

```bash
# Linux/macOS için otomatik kurulum / Automatic installation for Linux/macOS
cd ide
./install.sh

# Windows için / For Windows
cd ide
install.bat
```

Detaylı kurulum talimatları için bakınız / For detailed installation instructions, see:
👉 [ide/README.md](ide/README.md)

## 📚 Dil Özellikleri / Language Features

### Veri Tipleri / Data Types
- `INT` - Tam sayı / Integer
- `REAL` - Gerçek sayı / Real number
- `BOOL` - Boolean
- `CHAR` - Karakter / Character
- `POS` - Pozisyon / Position
- `E6POS` - Genişletilmiş pozisyon / Extended position
- `AXIS` - Eksen / Axis
- `FRAME` - Koordinat sistemi / Coordinate frame

### Kontrol Yapıları / Control Structures
- `IF...THEN...ELSE...ENDIF` - Koşullu ifadeler / Conditional statements
- `FOR...TO...ENDFOR` - For döngüsü / For loop
- `WHILE...ENDWHILE` - While döngüsü / While loop
- `SWITCH...CASE...ENDSWITCH` - Switch-case

### Hareket Komutları / Motion Commands
- `PTP` - Nokta-nokta hareket / Point-to-point motion
- `LIN` - Doğrusal hareket / Linear motion
- `CIRC` - Dairesel hareket / Circular motion
- `SPLINE` - Spline hareket / Spline motion

## 📝 Örnek Kod / Example Code

```grs
DEF ExampleProgram()
  ; Değişken tanımlama / Variable declaration
  DECL INT counter
  DECL REAL speed = 0.5
  DECL POS target
  
  ; Pozisyon atama / Position assignment
  target = {X 500, Y 0, Z 600, A 0, B 90, C 0}
  
  ; Ana pozisyona git / Go to home position
  PTP HOME
  
  ; Döngü / Loop
  FOR counter = 1 TO 5
    LIN target
    WAIT SEC 1.5
  ENDFOR
  
END
```

Daha fazla örnek için bakınız / For more examples, see:
- [ide/sample.grs](ide/sample.grs)
- [grs_interpreter/tests/](grs_interpreter/tests/)

## 🏗️ Proje Yapısı / Project Structure

```
grs-interpreter/
├── grs_interpreter/          # Ana yorumlayıcı / Main interpreter
│   ├── include/              # Header dosyaları / Header files
│   │   ├── lexer/           # Lexer bileşenleri / Lexer components
│   │   ├── parser/          # Parser bileşenleri / Parser components
│   │   ├── ast/             # AST tanımları / AST definitions
│   │   ├── interpreter/     # Yorumlayıcı / Interpreter
│   │   └── executor/        # Çalıştırıcı / Executor
│   ├── src/                 # Kaynak kodlar / Source code
│   ├── tests/               # Test dosyaları / Test files
│   └── CMakeLists.txt       # Build yapılandırması / Build config
│
├── ide/                      # IDE entegrasyonu / IDE integration
│   ├── zerobrane/           # ZeroBrane Studio dosyaları
│   ├── scintilla/           # Scintilla yapılandırmaları
│   ├── install.sh           # Linux/macOS kurulum scripti
│   ├── install.bat          # Windows kurulum scripti
│   └── README.md            # IDE dokümantasyonu / IDE documentation
│
└── examples/                 # Örnek projeler / Example projects
```

## 🧪 Testler / Tests

Test dosyaları `grs_interpreter/tests/` dizininde bulunur:

Test files are located in `grs_interpreter/tests/`:

- `executor_test.txt` - Executor testleri / Executor tests
- `example_krl_code.txt` - KRL kod örnekleri / KRL code examples
- `assign_and_type_convertion_test.txt` - Tip dönüşüm testleri / Type conversion tests
- `if_else_wait_test.txt` - Kontrol yapısı testleri / Control structure tests
- `pos_type_convertion.txt` - Pozisyon tip testleri / Position type tests

## �� Katkıda Bulunma / Contributing

Katkılarınızı bekliyoruz! / Contributions are welcome!

1. Fork yapın / Fork the repository
2. Feature branch oluşturun / Create a feature branch
   ```bash
   git checkout -b feature/amazing-feature
   ```
3. Değişikliklerinizi commit edin / Commit your changes
   ```bash
   git commit -m 'Add some amazing feature'
   ```
4. Branch'inizi push edin / Push your branch
   ```bash
   git push origin feature/amazing-feature
   ```
5. Pull Request oluşturun / Create a Pull Request

## 📄 Lisans / License

Bu proje açık kaynak kodludur. / This project is open source.

## 🔗 Bağlantılar / Links

- **Repository**: [github.com/FurkannByrm/grs-interpreter](https://github.com/FurkannByrm/grs-interpreter)
- **IDE Kurulumu**: [ide/README.md](ide/README.md)
- **Örnekler**: [examples/](examples/)

## 📧 İletişim / Contact

Sorularınız için GitHub Issues kullanabilirsiniz.

For questions, please use GitHub Issues.

---

⭐ Projeyi beğendiyseniz yıldız vermeyi unutmayın! / If you like this project, don't forget to give it a star!
