# Projet IHR




## PoseNet CNN


PoseNet détecte jusqu’à **17 points clés** sur le corps humain. Voici une description détaillée de chaque point clé et sa signification :


### **Liste des points détectés par PoseNet**

| **Index** | **Nom du point clé**       | **Description**                                               |
|-----------|----------------------------|---------------------------------------------------------------|
| 0         | **Nose**                   | Nez.                                                         |
| 1         | **Left Eye**               | Œil gauche.                                                  |
| 2         | **Right Eye**              | Œil droit.                                                   |
| 3         | **Left Ear**               | Oreille gauche.                                              |
| 4         | **Right Ear**              | Oreille droite.                                              |
| 5         | **Left Shoulder**          | Épaule gauche.                                               |
| 6         | **Right Shoulder**         | Épaule droite.                                               |
| 7         | **Left Elbow**             | Coude gauche.                                                |
| 8         | **Right Elbow**            | Coude droit.                                                 |
| 9         | **Left Wrist**             | Poignet gauche.                                              |
| 10        | **Right Wrist**            | Poignet droit.                                               |
| 11        | **Left Hip**               | Hanche gauche.                                               |
| 12        | **Right Hip**              | Hanche droite.                                               |
| 13        | **Left Knee**              | Genou gauche.                                                |
| 14        | **Right Knee**             | Genou droit.                                                 |
| 15        | **Left Ankle**             | Cheville gauche.                                             |
| 16        | **Right Ankle**            | Cheville droite.                                             |

10-8
8-6
6-5
6-12
5-11
5-7
7-9
11-12
12-14
14-16
11-13
13-15

---

### **Utilisation des points clés**
Les points clés détectés permettent de définir les parties du corps pour des applications variées comme :
1. **Gestes simples** : Lever une main, incliner le corps, etc.
2. **Suivi des mouvements** : Suivre la position des bras, des jambes ou du tronc.
3. **Calcul des angles** : Mesurer les angles entre les articulations pour des applications comme le fitness ou les soins de santé.
4. **Détection des postures** : Identifier des postures comme debout, assis, ou accroupi.

---

### **Exemple de visualisation**
Lorsque PoseNet détecte ces points clés, ils sont souvent représentés sous forme de cercles superposés sur l’image, reliés par des lignes pour illustrer les connexions du squelette.

#### **Connexions typiques**
PoseNet connecte certains points clés pour représenter le squelette :
- **Tête** : `Nose → Left Eye → Left Ear` et `Nose → Right Eye → Right Ear`
- **Tronc** : `Left Shoulder → Right Shoulder → Left Hip → Right Hip`
- **Bras gauche** : `Left Shoulder → Left Elbow → Left Wrist`
- **Bras droit** : `Right Shoulder → Right Elbow → Right Wrist`
- **Jambe gauche** : `Left Hip → Left Knee → Left Ankle`
- **Jambe droite** : `Right Hip → Right Knee → Right Ankle`

---