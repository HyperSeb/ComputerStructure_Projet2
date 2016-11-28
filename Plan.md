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
- un tableau avec les génomes des creatures
- un tableau pour retenir les couple indice des gènes et score
- une semaphore pour compter le nombre de créatures encore à traiter au cours d'une génération
- une semaphore pour compter le nombre de générations à produire

### Representation de la grille

On emballe la grille joliment dans une structure, ça permet de checker les indices, et par exemples de repondre 'obstacle' si on est en dehors. On y stocke si une case est vide (0), ou occupée pas un obstacle (1).

Les positions de départ et d'arrivée sont retenues séparément, pour y accéder facilement pour placer le pion au départ, ou calculer la distance jusqu'à l'arrivée

### Représentation des génomes

Une direction est une enum, un génome est un tableau de direction, voilà

## Processes

- P proccessus esclaves pour l'évaluation des creatures
- un processus interface qui s'occupe d'écouter sagement l'utilisateur, et transmet les infos au maître
- un processus maître, celui qui s'occupe de dire au processus exclaves quoi faire

### Maitre

Le maitre décrémente la semaphore des générations.
Pour une génération, il charge la queue (on est obligé d'utiliser une queue) avec tous les indices à traiter, et on incrémente la semaphore des créatures, ensuite attend qu'elle atteigne 0; puis trie le tableau des scores (!! le dernier doit rester le meilleur durant le tri)

### Esclave

Récupère un indice à traiter de la queue, crée un nouvelle créature à partir d'une des meilleurs, l'évalue, note son score, décrémente la semaphore

### Hal (celui qui discute avec l'utilisateur)

Attend l'utilisateur.
Si on lui demande des créer de nouvelles générations, il incrémente la sémaphore des générations.
Si on lui demande d'afficher le meilleur, il récupère l'info dans le dernier du tableau trié, et fais l'animation.
Si on lui demande de quitter, il fais le nettoyage et s'arrête.

## Trucs qu’on va devoir demander

- comment évaluer un score correct?
- peut-on utiliser une sémarphore en plus de la queue pour la comunication maitre-esclaves?
- peut-on vraiment créer les nouvelles créatures dans les esclaves?
