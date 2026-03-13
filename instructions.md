### Exécution des Benchmarks Automatisés

Pour évaluer et comparer les performances des différentes approches de parallélisation implémentées dans ce projet (Vectorisation + OpenMP, Première Façon - MPI Global, et Seconde Façon - MPI Décomposition de Domaine), un script de test automatisé a été intégré directement dans le Makefile.

Vous pouvez lancer la suite complète de benchmarks pour tester le programme sous différentes configurations matérielles (1, 2, 4, 8, 12 et 16 cœurs/threads) en exécutant simplement la commande suivante dans votre terminal :

make benchmark

Ce que fait cette commande :
* Configuration automatique : Elle ajuste les variables d'environnement (ex: OMP_NUM_THREADS=1 pour les tests MPI purs) pour éviter la surcharge matérielle (oversubscription).
* Boucle d'exécution : Elle lance la simulation itérativement pour chaque configuration de processus demandée.
* Génération de données : Elle crée automatiquement des répertoires organisés (test_X_cores/) contenant les fichiers .csv de profilage détaillés pour chaque itération, ainsi qu'un fichier global resume.csv compilant les temps finaux pour le calcul du Speedup.

Note : Assurez-vous d'avoir compilé le projet avec la commande `make` avant de lancer les benchmarks, et vérifiez que votre environnement supporte l'exécution MPI (`mpirun`).