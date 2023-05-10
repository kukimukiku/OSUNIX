# OSUNIX

OS UNIX kurso 3 užduotis (daugiaklientis serveris ir klientas).
Atliko informatikos 3 kurso studentė.

Parašytas daugiaklientis serveris sugeneruoja kažkurį žodį iš sąrašo, kurį spėlioja klientai (visi žaidžia kartu, t.y. spėlioja tą patį žodį vienu metu).

Programos paleidimas per terminalą:

Sukompiliavus kodą, pirmą paleidžiamas serveris:
  ./daugiaklientisserveris 8080
  
Tada prijungiami klientai:
  ./klientas 127.0.0.1 8080
  
Žaidimas prasideda kai kažkuris iš klientų spėja kokią nors raidę.
