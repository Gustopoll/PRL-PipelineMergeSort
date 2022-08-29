# PRL - Pipeline Merge Sort

#### Vypracoval: Dominik Švač
#### Rok: 2021

Pipeline Merge Sort je paralelný algoritmus. Je to riadiaci algoritmus, ktorý využíva log$_{2}$ (n)+1 procesorov. Procesory sú  lineárne prepojené pomocou dvoch liniek a obsahujú zoradené potupnosti hodnôt. Výnimkou je prvý procesor, ktorý  má iba jednu linku vstup a posledný procesor, ktorý má tiež iba jednu linku, na ktorej sa bude nachádzať výsledná zoradená postupnosť hodnôt.


### Požiadavky

Knižnica Open MPI

### Spustenie

```
./test.sh - vygeneruje dočastný súbor z náhodnými číslami a nastaví ich ako vstup pre program pms a ten ich zoradí pomocou algoritmu Pipeline Merge Sort
```
