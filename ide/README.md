# GRS IDE Kurulumu

GRS/KRL robot programlama dili için ZeroBrane Studio IDE desteği.

Syntax highlighting, autocomplete ve F5 ile program çalıştırma sağlar.

---

## Ön Gereksinimler

### 1. GRS Interpreter'ı Derle

```bash
cd grs_interpreter
mkdir -p build && cd build
cmake ..
make
```

Bu adım `grs_interpreter/build/interpreter` binary'sini oluşturur.

### 2. ZeroBrane Studio Kur

**Linux:**
```bash
# Proje klasöründeki installer'ı çalıştır
chmod +x ZeroBraneStudioEduPack-2.01-linux.sh
./ZeroBraneStudioEduPack-2.01-linux.sh
```
Varsayılan kurulum dizini: `/opt/zbstudio`

PATH'e ekle (opsiyonel):
```bash
echo 'export PATH="/opt/zbstudio:$PATH"' >> ~/.bashrc
source ~/.bashrc
```

**Windows:**
1. [ZeroBrane Studio](https://studio.zerobrane.com/download) sayfasından Windows installer'ı indir
2. Installer'ı çalıştır (varsayılan: `C:\ZeroBraneStudio`)
3. PATH'e eklemek istersen: Sistem > Ortam Değişkenleri > Path > `C:\ZeroBraneStudio` ekle

---

## GRS Dil Desteğini Kur

### Linux/macOS

```bash
cd ide
chmod +x install.sh
./install.sh
```

### Windows

```cmd
cd ide
install.bat
```

### Script ne yapıyor?

1. **4 dosyayı** ZeroBrane'in config dizinine kopyalar
2. **Interpreter symlink** oluşturur (`~/bin/interpreter`) — herhangi bir dizinden `.grs` dosyası çalıştırabilmeniz için
3. **`$HOME/bin`'i PATH'e** ekler (`.bashrc` veya `.zshrc` üzerinden)

| Dosya | Hedef | Görevi |
|-------|-------|--------|
| `grs.lua` | `~/.zbstudio/spec/` | Dil tanımı (lexer, keywords, renkler) |
| `grs.api` | `~/.zbstudio/api/grs/grs.lua` | Autocomplete tanımları |
| `grs_interpreter.lua` | `~/.zbstudio/interpreters/` | F5 ile çalıştırma |
| `grs-support.lua` | `~/.zbstudio/packages/` | Ana yükleyici (hepsini bağlar) |

> Windows'ta config dizini: `%USERPROFILE%\.zbstudio\`

---

## Kullanım

1. ZeroBrane Studio'yu aç:
   ```bash
   zbstudio ide/sample.grs
   ```

2. `.grs` dosyası açıldığında otomatik olarak:
   - GRS syntax highlighting aktif olur
   - GRS interpreter seçili gelir
   - Autocomplete çalışır (yazarken öneriler gösterir)

3. **F5** tuşuna basarak programı çalıştır

4. Çıktı alt paneldeki Output penceresinde görünür

### Herhangi Bir Dizinden Çalıştırma

Kurulum scripti interpreter binary'sini `~/bin/interpreter` olarak symlink eder ve `$HOME/bin`'i PATH'e ekler. Bu sayede `.grs` dosyalarını proje dizininin dışından da F5 ile çalıştırabilirsiniz.

Manuel kurulum:
```bash
mkdir -p ~/bin
ln -sf /tam/yol/grs_interpreter/build/interpreter ~/bin/interpreter
echo 'export PATH="$HOME/bin:$PATH"' >> ~/.bashrc
source ~/.bashrc
```

Interpreter şu sırayla aranır:
1. Açık dosyanın dizinine göre (`grs_interpreter/build/interpreter`, `../grs_interpreter/build/interpreter`)
2. ZeroBrane proje dizinine göre (aynı göreceli yollar)
3. `ide.config.path.grs` config değeri (user.lua'da ayarlanabilir)
4. Sistem PATH'i (`~/bin/interpreter` symlink bu adımda bulunur)

---

## Renk Şeması

| Renk | Keyword Grubu | Örnekler |
|------|---------------|----------|
| Mor | Program yapısı | `DEF` `END` |
| Kırmızımsı-mor (bold) | Kontrol akışı | `IF` `THEN` `ELSE` `ENDIF` `FOR` `WHILE` |
| Teal yeşil | Bildirimler & Veri tipleri | `DECL` `INT` `REAL` `BOOL` `POS` `AXIS` |
| Turuncu (bold) | Hareket komutları | `PTP` `LIN` `CIRC` `SPLINE` |
| Çelik mavisi | Sistem/I/O | `WAIT` `DELAY` `IN` `OUT` |
| Mavi (bold) | Sabitler & Mantık | `TRUE` `FALSE` `AND` `OR` `NOT` |
| Yeşil (italik) | Yorumlar | `; bu bir yorum` |
| Turuncu | Sayılar | `42` `3.14` |
| Gri | String'ler | `"hello"` |

---

## Dosya Yapısı

```
ide/
├── install.sh            # Linux/macOS kurulum scripti
├── install.bat           # Windows kurulum scripti
├── sample.grs            # Örnek GRS programı
├── README.md             # Bu dosya
├── zerobrane/            # ZeroBrane Studio dosyaları
│   ├── grs.lua           # Dil tanımı (spec)
│   ├── grs.api           # Autocomplete (API)
│   ├── grs_interpreter.lua  # Interpreter plugin
│   └── grs-support.lua   # Package loader
└── scintilla/            # Notepad++ / SciTE dosyaları
    ├── grs-language.xml   # Notepad++ UDL import
    └── grs.properties     # SciTE properties
```

---

## Nasıl Çalışıyor (Teknik Detay)

### Problem

ZeroBrane Studio yeni bir dil eklemek için `~/.zbstudio/spec/` dizinine dosya koymanız yetmez.
IDE bu dizinleri **otomatik yüklemez**. Ayrıca `user.lua` dosyası tüm editor modüllerinden **önce** yüklenir,
bu yüzden `ide:LoadSpec()` gibi fonksiyonlar `user.lua` içinde çağrıldığında hata verir
(örn. `ReloadAPIs nil` hatası — `autocomplete.lua` modülü henüz yüklenmemiş olur).

### Çözüm: Package Sistemi

ZeroBrane'in **package** sistemi (`~/.zbstudio/packages/`) tüm editor modüllerinden **sonra** yüklenir.
`grs-support.lua` dosyası bir package olarak:

1. `ide:AddSpec("grs", spec)` — Dil tanımını yükler
2. `ide:AddInterpreter("grs", interp)` — Interpreter'ı kaydeder
3. `ide:AddAPI("grs", "grs", api)` — Autocomplete'i aktif eder
4. `ide.config.interpreter = "grs"` — GRS'i varsayılan interpreter yapar
5. `ide.config.styles.keywords0-5` — Renkleri özelleştirir

### Lexer Seçimi: ASM

GRS dili `;` ile yorum yapıyor. Scintilla'nın yerleşik lexer'ları arasında bunu destekleyen
**ASM lexer** (`wxSTC_LEX_ASM = 34`) kullanılıyor. ASM lexer 6 farklı keyword grubu destekliyor:

| Style ID | Scintilla Adı | GRS Kullanımı |
|----------|---------------|---------------|
| 6 | `SCE_ASM_CPUINSTRUCTION` | `DEF` `END` |
| 7 | `SCE_ASM_MATHINSTRUCTION` | `IF` `FOR` `WHILE` ... |
| 8 | `SCE_ASM_REGISTER` | `DECL` `INT` `POS` ... |
| 9 | `SCE_ASM_DIRECTIVE` | `PTP` `LIN` `CIRC` ... |
| 10 | `SCE_ASM_DIRECTIVEOPERAND` | `WAIT` `DELAY` `IN` `OUT` |
| 14 | `SCE_ASM_EXTINSTRUCTION` | `TRUE` `FALSE` `AND` ... |

> **Önemli:** ASM lexer keyword eşleştirmesi yaparken metni küçük harfe çevirir (`GetCurrentLowered`).
> Bu yüzden spec dosyasındaki keyword'ler **küçük harf** olmalıdır (`def end` — `DEF END` değil).
> Lexer case-insensitive çalıştığı için koddaki `DEF` de `def` de renklendirilir.

---

## Notepad++ Kurulumu (Opsiyonel)

1. Notepad++ aç
2. **Dil > Kendi Dilinizi Tanımlayın > İçe Aktar**
3. `ide/scintilla/grs-language.xml` dosyasını seç
4. Notepad++'ı yeniden başlat

## SciTE Kurulumu (Opsiyonel)

```bash
cp ide/scintilla/grs.properties ~/.scite/
echo "import grs" >> ~/.SciTEUser.properties
```

---

## Sorun Giderme

| Sorun | Çözüm |
|-------|-------|
| F5 basılmıyor (gri) | Interpreter seçili mi kontrol et: Project > Interpreter > GRS |
| "GRS Interpreter not found!" | `grs_interpreter/build/interpreter` binary'si var mı? `cmake .. && make` ile derle |
| Renkler görünmüyor | ZeroBrane'i kapat, `./install.sh` tekrar çalıştır, yeniden aç |
| Lua hatası (syntax error) | GRS dosyası Lua interpreterle çalıştırılıyor. Project > Interpreter > GRS seç |
| Autocomplete çalışmıyor | `.grs` uzantılı dosya açın, yazarken CTRL+Space deneyin |
| Farklı dizindeki .grs çalışmıyor | `~/bin/interpreter` symlink var mı? `ls -la ~/bin/interpreter` ile kontrol edin |
