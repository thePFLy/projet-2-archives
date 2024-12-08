# projet-2-archives
concepts inhérents aux systèmes de fichiers. archives tar et à leur format.

https://www.gnu.org/software/tar/manual/html_node/Standard.html

## Test locaux:
tar --posix --pax-option delete=".*" --pax-option delete="*time*" --no-xattrs --no-acl --no-selinux -c fichier1 fichier2 ... >   archive.tar

## Consignes:
Vous avez vu au dernier cours les concepts inhérents aux systèmes de fichiers. Le projet que vous aurez à réaliser utilisera plusieurs de ces concepts. Nous nous intéresserons aux archives tar et à leur format. Toutes les informations sur le format utiles à la réalisation du projet sont disponibles au lien suivant. Votre objectif sera d'implémenter plusieurs fonctions qui manipulent une archive tar.

Un squelette du projet avec les spécifications des fonctions à implémenter est disponible ici. Elle contient :
- Un fichier header lib_tar.h avec la définition de la structure POSIX utilisée par Tar et les signatures des fonctions à implémenter.
- Un fichier lib_tar.c où vous écrirez vos implémentations.
- Un fichier tests.c que vous utiliserez pour écrire des tests.
- Un Makefile. La cible submit (make submit) permet de préparer une archive de votre projet pour la soumission. C'est cette archive que vous uploaderez sur la tâche INGInious.

