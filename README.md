# Házi feladat

Név/Nevek NEPTUN kóddal:
- Ondrejó András (XTSR1F)

# Feladat specifikáció
Webalkalmazás backend írást segítő library (~~HTTP Client és~~ Szerver) és mintaprogram

Go (net/http) és nodejs mintájára kényelmes lehet ha több URI kezelését és a statikus startalom kiszolgálását is egy alkalmazáson belül valósíthatom meg.
Erre a célra library-m a következő eszközöket nyújtaná:
* HTTP headerek értelmezése, hogy C++-ból könnyen használható formában legyen.
* IO multiplexálás hálózati kommunikációhoz és fájl műveletekhez.
* A HTTP végpontokat kezelő függvények megvalósításához coroutine-ok, hogy szekvenciális kódot lehessen írni blokkoló műveletekkel.
* URI alapján megfelelő végpont kiválasztása. pl.: /articles/{id} egy felhasználó által definiált függvény, / pedig sttaikus fájlkiszolgálás
* Bizonyos kérésekhez ~~middleware-ek beszúrása.~~ pl.: /articles PUT metódus esetén basicAuth-ot igényel ~~akkor ezt nem a kérést kiszolgáló függvények kell kezelnie~~

# Eredmény
https://drive.google.com/file/d/1VKLJ2RqHuOequHI2asJqfhP6yk0mOWzH/view?usp=sharing

Minta alkalmazás: Tic-tac-toe

* HTTP Client nem lett megvalósítva (komolyabb módosítások nélkül lehetséges)
* Middleware nincs, a http::handle-ból kell kezelni mindent
* A nem magától értetődő részek kommentelve, kivétel a stringekben keresgélés