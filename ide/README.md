# GRS/KRL IDE Setup Guide

Bu dizin, GRS/KRL robot programlama dili için IDE entegrasyonu dosyalarını içerir. ZeroBrane Studio ve Scintilla tabanlı editörler için dil desteği sağlar.

This directory contains IDE integration files for the GRS/KRL robot programming language. It provides language support for ZeroBrane Studio and Scintilla-based editors.

## İçindekiler / Contents

- **zerobrane/** - ZeroBrane Studio entegrasyon dosyaları
- **scintilla/** - Scintilla lexer konfigürasyonları

## ZeroBrane Studio Kurulumu / ZeroBrane Studio Setup

### Gereksinimler / Requirements
- ZeroBrane Studio 1.0 veya üzeri / 1.0 or higher
- GRS Interpreter (derlenmiş) / GRS Interpreter (compiled)

### Kurulum Adımları / Installation Steps

1. **ZeroBrane Studio'yu İndirin ve Kurun**
   ```bash
   # Linux
   wget https://download.zerobrane.com/ZeroBraneStudioEduPack-X.XX-linux.sh
   sh ZeroBraneStudioEduPack-X.XX-linux.sh
   
   # Windows
   # Download from https://studio.zerobrane.com/download
   ```

2. **GRS Dil Spesifikasyonunu Kopyalayın**
   ```bash
   # ZeroBrane Studio spec dizinini bulun
   # Linux: ~/.zbstudio/spec/
   # Windows: %USERPROFILE%\.zbstudio\spec\
   
   cp ide/zerobrane/grs.lua ~/.zbstudio/spec/
   ```

3. **API Dosyasını Kopyalayın**
   ```bash
   # ZeroBrane Studio api dizinini bulun
   # Linux: ~/.zbstudio/api/
   # Windows: %USERPROFILE%\.zbstudio\api\
   
   cp ide/zerobrane/grs.api ~/.zbstudio/api/
   ```

4. **ZeroBrane Studio'yu Yeniden Başlatın**
   - Artık .grs, .krl, .src, .dat uzantılı dosyaları açabilirsiniz
   - Syntax highlighting ve autocomplete çalışacaktır

### Kullanım / Usage

1. ZeroBrane Studio'yu açın
2. Bir .grs veya .krl dosyası açın
3. Kod editörünüzde syntax highlighting göreceksiniz
4. Ctrl+Space ile autocomplete menüsünü açabilirsiniz
5. F5 ile programı çalıştırabilirsiniz (interpreter kuruluysa)

## Scintilla Tabanlı Editörler / Scintilla-Based Editors

### Notepad++ (Windows)

1. **Notepad++'ı Açın**

2. **Dil Dosyasını İçe Aktarın**
   - Menü: Language → User Defined Language → Define your language...
   - "Import..." butonuna tıklayın
   - `ide/scintilla/grs-language.xml` dosyasını seçin
   - Notepad++'ı yeniden başlatın

3. **Kullanım**
   - .grs, .krl, .src, .dat dosyalarını açın
   - Syntax highlighting otomatik olarak uygulanacaktır

### SciTE (Cross-platform)

1. **Properties Dosyasını Kopyalayın**
   ```bash
   # Linux
   cp ide/scintilla/grs.properties ~/.SciTEUser.properties
   # veya sistem geneli için
   sudo cp ide/scintilla/grs.properties /usr/share/scite/grs.properties
   
   # Windows
   # Copy to %USERPROFILE%\SciTEUser.properties
   ```

2. **Ana Properties Dosyasını Düzenleyin**
   
   SciTEUser.properties veya SciTEGlobal.properties dosyasına ekleyin:
   ```
   import grs
   ```

3. **SciTE'yi Yeniden Başlatın**

### Geany

1. **Filetype Tanımlaması Oluşturun**
   ```bash
   mkdir -p ~/.config/geany/filedefs
   cp ide/scintilla/grs.properties ~/.config/geany/filedefs/filetypes.GRS.conf
   ```

2. **Dosya Tipini Yapılandırın**
   
   `~/.config/geany/filetype_extensions.conf` dosyasına ekleyin:
   ```
   GRS=*.grs;*.krl;*.src;*.dat;
   ```

3. **Geany'yi Yeniden Başlatın**

## Özellikler / Features

### Syntax Highlighting
- ✅ Kontrol yapıları (IF, FOR, WHILE, SWITCH)
- ✅ Veri tipleri (INT, REAL, BOOL, POS, vb.)
- ✅ Hareket komutları (PTP, LIN, CIRC, vb.)
- ✅ Yerleşik fonksiyonlar
- ✅ Yorumlar (;)
- ✅ String ve sayı değerleri
- ✅ Boolean değerler ve operatörler

### Autocomplete (ZeroBrane Studio)
- ✅ Anahtar kelimeler
- ✅ Veri tipleri
- ✅ Yerleşik fonksiyonlar
- ✅ Fonksiyon parametreleri
- ✅ Kod snippet'leri

### Code Folding
- ✅ Fonksiyon tanımları (DEF...ENDDEF)
- ✅ Koşullu bloklar (IF...ENDIF)
- ✅ Döngüler (FOR...ENDFOR, WHILE...ENDWHILE)
- ✅ Switch blokları (SWITCH...ENDSWITCH)

### İnterpreter Entegrasyonu (ZeroBrane Studio)
- ✅ F5 ile kod çalıştırma
- ✅ Hata mesajları
- ✅ Çıktı konsolu

## Desteklenen Dosya Uzantıları / Supported File Extensions

- `.grs` - GRS kaynak dosyaları
- `.krl` - KRL kaynak dosyaları
- `.src` - Kaynak dosyaları
- `.dat` - Veri dosyaları

## Dil Özellikleri / Language Features

### Anahtar Kelimeler / Keywords

**Kontrol Yapıları:**
- `DEF`, `ENDDEF` - Fonksiyon tanımlama
- `IF`, `THEN`, `ELSE`, `ENDIF` - Koşullu ifadeler
- `FOR`, `TO`, `STEP`, `ENDFOR` - For döngüsü
- `WHILE`, `ENDWHILE` - While döngüsü
- `SWITCH`, `CASE`, `DEFAULT`, `ENDSWITCH` - Switch-case
- `RETURN`, `CONTINUE`, `BREAK` - Kontrol akışı

**Veri Tipleri:**
- `INT` - Tam sayı
- `REAL` - Gerçek sayı
- `BOOL` - Boolean
- `CHAR` - Karakter
- `POS` - Pozisyon (X, Y, Z, A, B, C)
- `E6POS` - Genişletilmiş pozisyon
- `AXIS` - Eksen pozisyonu
- `E6AXIS` - Genişletilmiş eksen pozisyonu
- `FRAME` - Koordinat sistemi
- `STRUC` - Yapı
- `ENUM` - Sıralama

**Hareket Komutları:**
- `PTP` - Nokta-nokta hareket
- `LIN` - Doğrusal hareket
- `CIRC` - Dairesel hareket
- `SPLINE` - Spline hareket
- `LIN_REL` - Göreceli doğrusal hareket
- `PTP_REL` - Göreceli nokta-nokta hareket
- `HOME` - Ana pozisyon

**Yerleşik Fonksiyonlar:**
- Matematiksel: `SQRT`, `SIN`, `COS`, `TAN`, `ABS`, `POW`, `EXP`, `LOG`, `MIN`, `MAX`
- String: `STRLEN`, `STRFIND`, `STRCOMP`
- Robot Kontrolü: `SET_PTP_VEL`, `SET_LIN_VEL`, `BRAKE_ON`, `BRAKE_OFF`, `STOP_ROBOT`
- Pozisyon: `GET_POS`, `GET_AXIS`, `SET_TOOL`, `SET_BASE`

## Örnek Kod / Example Code

```grs
DEF ExampleProgram()
  ; Bu bir örnek programdır
  
  DECL INT counter
  DECL REAL speed = 0.5
  DECL BOOL flag
  
  ; Ana pozisyona git
  PTP HOME
  
  FOR counter = 1 TO 5
    IF flag == FALSE THEN
      LIN {X 500, Y 0, Z 600, A 0, B 90, C 0}
    ELSE
      LIN_REL {Z -50}
    ENDIF
    
    WAIT SEC 1.5
    flag = NOT flag
  ENDFOR
  
END

DEF REAL calculate_distance(REAL x1, REAL y1, REAL x2, REAL y2)
  DECL REAL result
  result = SQRT((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1))
  RETURN result
ENDDEF
```

## Sorun Giderme / Troubleshooting

### ZeroBrane Studio'da syntax highlighting çalışmıyor
1. Dosyaların doğru dizine kopyalandığından emin olun
2. ZeroBrane Studio'yu tamamen kapatıp yeniden açın
3. Dosya uzantısının .grs, .krl, .src veya .dat olduğundan emin olun

### Autocomplete çalışmıyor
1. .api dosyasının doğru yere kopyalandığından emin olun
2. Ctrl+Space tuş kombinasyonunu kullanın
3. ZeroBrane Studio ayarlarında autocomplete'in açık olduğunu kontrol edin

### Interpreter çalışmıyor
1. GRS interpreter'ın derlendiğinden emin olun
2. grs.lua dosyasındaki interpreter_path yolunu güncelleyin
3. Çalıştırma izinlerini kontrol edin

## Katkıda Bulunma / Contributing

IDE desteğini geliştirmek için katkılarınızı bekliyoruz:
- Yeni özellikler ekleyin
- Hataları düzeltin
- Dokümantasyonu iyileştirin
- Yeni editör desteği ekleyin

## Lisans / License

Bu IDE entegrasyon dosyaları GRS Interpreter projesi ile aynı lisans altındadır.

## Kaynaklar / Resources

- [ZeroBrane Studio Documentation](https://studio.zerobrane.com/doc-general-preferences)
- [Scintilla Documentation](https://www.scintilla.org/ScintillaDoc.html)
- [GRS Interpreter Repository](https://github.com/FurkannByrm/grs-interpreter)
