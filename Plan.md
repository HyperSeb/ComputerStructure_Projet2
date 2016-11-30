# Plan

## Files de messages

- une file entre le maitre et les esclaves, pour transmettre les indices des créatures à évaluer, et dans l'autre sens ceux des créatures qui viennent d'être évaluées

## Sémaphores

- une sémaphore pour l'excusivité pour l'indice du meilleur score
- une sémaphore pour compter le nombre de générations à produire

## Mémoire partagée

- indice du meilleur (taille:	1)
- tableau des scores (taille:	nombre de créatures (C))
- tableau des genomes (taille: C * nombre de déplacements (T))
- grille (taille: hauteur (M) * largeur (N))
- position du départ (taille: 2)
- position du arrivée (taille: 2)

### Representation de la grille

On emballe la grille joliment dans une structure, ça permet de checker les indices, et par exemples de repondre 'obstacle' si on est en dehors. On y stocke si une case est vide (0), ou occupée pas un obstacle (1).

Les positions de départ et d'arrivée sont retenues séparément, pour y accéder facilement pour placer le pion au départ, ou calculer la distance jusqu'à l'arrivée

### Représentation des génomes

Une direction est une enum, un génome est un tableau de direction, voilà

## Autres arguments

- hauteur (M)
- largeur (N)
- nombre de déplacements (T)
- nombre d'esclaves (P)
- nombre de créatures (C)
- taux de suppression (p)
- taux de mutation (m)
  
## Processus

- un processus interface (Hal) qui s'occupe d'écouter sagement l'utilisateur
- un processus maître, celui qui s'occupe de dire aux processus exclaves quoi faire
- P proccessus esclaves pour l'évaluation des créatures

### Hal

C'est celui lancé par l'utilisateur avec les arguments, il doit allouer la mémoire partagée, créer les sémaphores et la file de messages, ensuite créer le maitre.

Ensuite il attend l'utilisateur, et on lui demande soit:
- de créer de nouvelles générations, alors il incrémente la sémaphore des générations.
- d'afficher le meilleur, il récupère l'info dans la mémoire partagée, et fais l'animation.
- de quitter, il fais le nettoyage et s'arrête.

### Maitre

Il crée les esclaves.

Il décrémente la semaphore des générations. Pour la première fois, il doit créer tous les génomes, il envoie au fur et à mesure les indices dans la file de messages.
Il crée une file a priorité, dont les éléments prioritaires sont ceux dont le score est le plus faible.
Il récupère progressivement les indices des créatures évaluées de la file de messages, et les ajoute dans la file à priorité.

Il entre ensuite dans une boucle: 
- décrémente la sémaphore des générations
- extrait les moins bonnes créatures de la file à priorité
- crée pour chacune de ces créatures un nouveau génome, et l'envoie dans la file de messages aux esclaves
- récupère les indices des créatures évaluées, et les ajoute dans la file à priorité

### Esclave

Entre directement dans un boucle:
- récupère l'indice d'une créature à évaluer dans la file de messages
- évalue la créature
- inscrit son score dans la mémoire partagée
- renvoie l'indice au maitre
- actualise l'indice de la meilleure créature si nécessaire, en s'assurant de l'exclusivité avec la sémaphore

