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
- les "gènes" des creatures (leurs mouvements)
- l’index du « nombre de creatures modifiées » // what ?
- int best one so far // ça oui, mais pas que dans la shared memory, si?
- semaphores // pour ?

### Representation de la grille

Je propose qu'on emballe la grille joliment dans une structure, ça permet de checker les indices, et par exemples de repondre 'obstacle' si on est en dehors (c'était une blague le #UncheckedMasterRace)

Je propose d'y stocker si une case est libre, ou si c'est un obstacle, l'arrivée, le départ, et je propose comme valeur: 0 -> libre, 1 -> obstacle, 2 -> départ, 3 -> arrivée (ou bien on fait une enum si on veut être propre). Pas sur que ce soit intéressant d'y stocker le départ, mais pour l'arrivée je crois que c'est le plus simple il suffit de voir si la case occupée est l'arrivée -> on est arrivé

### Représentation des gènes

En gros un paquet de char (UInt8) dans la mémoire partagée (peut-être suivi de char inutilisé si il faut un alignement particulier), ou bien on peut faire plus compact puisque un mouvement n'a que 8 posibilités, ça ne prend que 3 bits, donc on peut en mettre 2 dans un char, ou 5 dans un UInt16, ou 10 dans un UInt32 (ok c'est pas mieux qu'avec UInt16), ou 21 dans un UInt64, et du coup l'emballer proprement, sinon c'est dégueulasse. (si on fait ça je pense que le mieux est des UInt16 (ou UInt8) qui me semble avoir le meilleur rapport densité / espace inutilisé (si on nous dit 9 mouvements, on a 1 inutilisé à la fin))

## Processes

- P proccessus esclaves pour l'évaluation des creatures
- un processus interface qui s'occupe d'écouter sagement l'utilisateur, et transmet les infos au maître
- un processus maître, celui qui s'occupe de dire au processus exclaves quoi faire (avec les queues (j'imagine que c'est là qu'on s'en sers)), et qui gère la création de plusieurs générations, le tri, et autre chose du genre

## Trucs qu’on va devoir demander

- est-ce qu'on choisis les commandes à taper? ou bien elles sont fixées mais pas indiquées dans l'énoncé car celui qui l'a fait les a oubliées / intentionellement omises pour perturber des élèves / elles y sont mais écrites en tout petits, dans un coin, en blanc, et en grec?
- est-ce qu'on peut considérer seulement 7 possibilités pour les mouvements car il y a forcément un obstacle sous la créature quand elle peut bouger
