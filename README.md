# Tema1APD-Mandelbrot_Julia
[Tema1 Algoritmi Paraleli si Distribuiti (2020-2021, seria CB)] 

Generare paralela de fractali folosind multimile Mandelbrot si Julia. Tema presupune paralelizarea celor 2 algoritmi. <br>
Enunt: https://curs.upb.ro/pluginfile.php/391650/mod_resource/content/3/Tema1.pdf

#### COMPILARE SI RULARE
      SECVENTIAL (in directorul skel)
          ► make
          ► ./tema1 <input_julia> <output_julia> <input_mandelbrot> <output_mandelbrot>
      PARALEL (in directorul sol)
          ► make
          ► ./tema1_par <input_julia> <output_julia> <input_mandelbrot> <output_mandelbrot> <P>

#### IMPLEMENTARE

Pentru a putea rula algoritmii unul dupa altul, am declarat 2 matrici result
(result_m si result_j) care retin informatiile citite din fisier.
Am creeat si pornit thread-urile folosind functia "thread_function" care
calculeaza in functie de id-ul thread-ului capetele intervalelor pe care 
va rula fiecare thread. Pentru paralelizarea primului for din algoritmi,
am calculat start1 si end1, astfel incat intervalul [0, weight] sa fie 
impartit egal intre thread-uri. Pentru paralelizarea for-ului care transforma
coordonatele matematice in coordonate ecran, am calculat in mod asemanator
start2 si end2, care impart intervalul [0, height/2].

Am adaugat bariere unde era nevoie(de exemplu inainte de transformarea
coordonatelor pentru a fi sigura ca matricea result este calculata complet).
Un procedeu asemanator am facut si pentru Mandelbrot, prin recalcularea
intervalelor. 

Cand thread-urile au terminat calcularea, se da join si se scrie in fisier
rezultatul obtinut pentru ambele matrici, dupa care se elibereaza memoria.
