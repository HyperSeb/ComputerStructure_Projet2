# Plan

## Shared memory

- les données du programme
  - grille (si on l'emballe on y retient height (M) et witdh (N), et peut-être départ)
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
- semaphores // pour ?

### Representation de la grille

On emballe la grille joliment dans une structure, ça permet de checker les indices, et par exemples de repondre 'obstacle' si on est en dehors

Je propose d'y stocker si une case est libre, ou si c'est un obstacle, l'arrivée, le départ, et je propose comme valeur: 0 -> libre, 1 -> obstacle, 2 -> départ, 3 -> arrivée (ou bien on fait une enum si on veut être propre). Pas sur que ce soit intéressant d'y stocker le départ, mais pour l'arrivée je crois que c'est le plus simple il suffit de voir si la case occupée est l'arrivée -> on est arrivé

### Représentation des gènes

Dans la mémoire partagée:
- soit T char (UInt8)
- soit T/2 UInt8 ou T/5 UInt16 comme 8 possibilités -> 3 bits

(suivi si nécessaire de vide pour aligner le reste)

## Processes

- P proccessus esclaves pour l'évaluation des creatures
- un processus interface qui s'occupe d'écouter sagement l'utilisateur, et transmet les infos au maître
- un processus maître, celui qui s'occupe de dire au processus exclaves quoi faire (avec les queues (j'imagine que c'est là qu'on s'en sers)), et qui gère la création de plusieurs générations, le tri, et autre chose du genre

## Trucs qu’on va devoir demander

- est-ce qu'on choisis les commandes à taper? ou bien elles sont fixées mais pas indiquées dans l'énoncé car celui qui l'a fait les a oubliées / intentionellement omises pour perturber des élèves / elles y sont mais écrites en tout petits, dans un coin, en blanc, et en grec?
- est-ce qu'on peut considérer seulement 7 possibilités pour les mouvements car il y a forcément un obstacle sous la créature quand elle peut bouger
- est-ce que toutes les créatures mutent?
