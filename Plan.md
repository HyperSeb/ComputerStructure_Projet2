# Plan

## Shared memory

- les données du programme
  - grille (avec height (M) et witdh (N))
  - départ
  - arrivée
  - number of processes (P)
  - number of creature (C)
  - deletion rate (p)
  - mutation rate (m)
  - number of moves (T))
- les "gènes" des creatures (leurs mouvements), et leur score
- les gènes du meilleur (pas sur voir question à la fin)
- l’index de la prochaine créature à évaluer
- semaphores gérer les exclusions

### Representation de la grille

On emballe la grille joliment dans une structure, ça permet de checker les indices, et par exemples de repondre 'obstacle' si on est en dehors. On y stocke si une case est vide (0), ou occupée pas un obstacle (1).

Les positions de départ et d'arrivée sont retenues séparément, pour y accéder facilement pour placer le pion au départ, ou calculer la distance jusqu'à l'arrivée

### Représentation des gènes

Des char (Int8) dans la mémoire partagée. Des constantes pour les directions

-OU-

une enum pour les directions

## Processes

- P proccessus esclaves pour l'évaluation des creatures
- un processus interface qui s'occupe d'écouter sagement l'utilisateur, et transmet les infos au maître
- un processus maître, celui qui s'occupe de dire au processus exclaves quoi faire (avec les queues (j'imagine que c'est là qu'on s'en sers)), et qui gère la création de plusieurs générations, le tri, et autre chose du genre

## Trucs qu’on va devoir demander

- est-ce qu'on choisis les commandes à taper? ou bien elles sont fixées mais pas indiquées dans l'énoncé car celui qui l'a fait les a oubliées / intentionellement omises pour perturber des élèves / elles y sont mais écrites en tout petits, dans un coin, en blanc, et en grec?
- est-ce qu'on peut considérer seulement 7 possibilités pour les mouvements car il y a forcément un obstacle sous la créature quand elle peut bouger
- est-ce que toutes les créatures mutent?
