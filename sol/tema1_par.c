/*
 * APD - Tema 1
 * Octombrie 2020
 * Gherasie Stefania 333CB
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>

char *in_filename_julia;
char *in_filename_mandelbrot;
char *out_filename_julia;
char *out_filename_mandelbrot;
int P;
int **result_j;
int **result_m;
pthread_barrier_t barrier;
#define min(a,b) (((a)<(b))?(a):(b))

// structura pentru un numar complex
typedef struct _complex {
	double a;
	double b;
} complex;

// structura pentru parametrii unei rulari
typedef struct _params {
	int is_julia, iterations;
	double x_min, x_max, y_min, y_max, resolution;
	complex c_julia;
} params;

int width_j, width_m, height_j, height_m;
params par_j, par_m;

// citeste argumentele programului
void get_args(int argc, char **argv)
{
	if (argc < 6) {
		printf("Numar insuficient de parametri:\n\t"
				"./tema1 fisier_intrare_julia fisier_iesire_julia "
				"fisier_intrare_mandelbrot fisier_iesire_mandelbrot "
				"numar_threaduri\n");
		exit(1);
	}

	in_filename_julia = argv[1];
	out_filename_julia = argv[2];
	in_filename_mandelbrot = argv[3];
	out_filename_mandelbrot = argv[4];
	P = atoi(argv[5]);
}

// citeste fisierul de intrare
void read_input_file(char *in_filename, params* par)
{
	FILE *file = fopen(in_filename, "r");
	if (file == NULL) {
		printf("Eroare la deschiderea fisierului de intrare!\n");
		exit(1);
	}

	fscanf(file, "%d", &par->is_julia);
	fscanf(file, "%lf %lf %lf %lf",
			&par->x_min, &par->x_max, &par->y_min, &par->y_max);
	fscanf(file, "%lf", &par->resolution);
	fscanf(file, "%d", &par->iterations);

	if (par->is_julia) {
		fscanf(file, "%lf %lf", &par->c_julia.a, &par->c_julia.b);
	}

	fclose(file);
}

// scrie rezultatul in fisierul de iesire
void write_output_file(char *out_filename, int **result, int width, int height)
{
	int i, j;

	FILE *file = fopen(out_filename, "w");
	if (file == NULL) {
		printf("Eroare la deschiderea fisierului de iesire!\n");
		return;
	}

	fprintf(file, "P2\n%d %d\n255\n", width, height);
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			fprintf(file, "%d ", result[i][j]);
		}
		fprintf(file, "\n");
	}

	fclose(file);
}

// aloca memorie pentru rezultat
int **allocate_memory(int width, int height)
{
	int **result;
	int i;

	result = malloc(height * sizeof(int*));
	if (result == NULL) {
		printf("Eroare la malloc!\n");
		exit(1);
	}

	for (i = 0; i < height; i++) {
		result[i] = malloc(width * sizeof(int));
		if (result[i] == NULL) {
			printf("Eroare la malloc!\n");
			exit(1);
		}
	}

	return result;
}

// elibereaza memoria alocata
void free_memory(int **result, int height)
{
	int i;

	for (i = 0; i < height; i++) {
		free(result[i]);
	}
	free(result);
}

// ruleaza algoritmul Julia 
void run_julia(params *par, int **result, int start1, int end1, int start2, int end2)
{
	int w, h, i;

	// Paralelizarea obtinerii coordonatelor matematice
	// Fiecare thread calculeaza coordonatele in intervalul asociat id-ului lor
	for (w = start1; w <= end1; w++) {
		for (h = 0; h < height_j; h++) {
			int step = 0;
			complex z = { .a = w * par->resolution + par->x_min,
							.b = h * par->resolution + par->y_min };

			while (sqrt(pow(z.a, 2.0) + pow(z.b, 2.0)) < 2.0 && step < par->iterations) {
				complex z_aux = { .a = z.a, .b = z.b };

				z.a = pow(z_aux.a, 2) - pow(z_aux.b, 2) + par->c_julia.a;
				z.b = 2 * z_aux.a * z_aux.b + par->c_julia.b;

				step++;
			}

			result[h][w] = step % 256;
		}
	}

	// Bariera ca thread-urile sa inceapa transformarea coordonatelor dupa ce 
	// toate au terminat de calculat partea asociata din matrice.
	pthread_barrier_wait(&barrier);

	// Transforma rezultatul din coordonate matematice in coordonate ecran
	// Fiecare thread calculeaza coordonatele in intervalul asociat id-ului lor
	for (i = start2; i <= end2; i++) {
		int *aux = result[i];
		result[i] = result[height_j - i - 1];
		result[height_j - i - 1] = aux;
	}
}


// ruleaza algoritmul Mandelbrot
void run_mandelbrot(params *par, int **result, int start1, int end1, int start2, int end2)
{
	int w, h, i;

	// Paralelizarea obtinerii coordonatelor matematice
	// Fiecare thread calculeaza coordonatele in intervalul asociat id-ului lor
	for (w = start1; w <= end1; w++) {
		for (h = 0; h < height_m; h++) {
			complex c = { .a = w * par->resolution + par->x_min,
							.b = h * par->resolution + par->y_min };
			complex z = { .a = 0, .b = 0 };
			int step = 0;

			while (sqrt(pow(z.a, 2.0) + pow(z.b, 2.0)) < 2.0 && step < par->iterations) {
				complex z_aux = { .a = z.a, .b = z.b };

				z.a = pow(z_aux.a, 2.0) - pow(z_aux.b, 2.0) + c.a;
				z.b = 2.0 * z_aux.a * z_aux.b + c.b;

				step++;
			}

			result[h][w] = step % 256;
		}
	}

	// Bariera ca thread-urile sa inceapa transformarea coordonatelor dupa ce 
	// toate au terminat de calculat partea asociata din matrice.
	pthread_barrier_wait(&barrier);


	// Transforma rezultatul din coordonate matematice in coordonate ecran
	// Fiecare thread calculeaza coordonatele in intervalul asociat id-ului lor
	for (i = start2; i <= end2; i++) {
		int *aux = result[i];
		result[i] = result[height_m - i - 1];
		result[height_m - i - 1] = aux;
	}
}



void *thread_function(void *arg)
{
	// Se obtine id-ul thread-ului curent
	int thread_id = *(int *)arg;

	// Se calculeaza capetele intervalului fiecarui thread pentru paralelizarea
	// primului for din algoritmul Julia (for 0, width)
	int start1 = thread_id* (double)width_j / P ; 
    int end1 = min((thread_id + 1) * (double)width_j / P  - 1, width_j);
	// Se calculeaza capetele intervalului fiecarui thread pentru paralelizarea
	// for-ului care transforma coordonatele (for 0, height/2)
    int start2 = thread_id* (double)(height_j / 2) / P;
    int end2 = min((thread_id + 1) * (double)(height_j / 2) / P  - 1, (height_j / 2));


    // Se ruleaza algoritmul Julia
    run_julia(&par_j, result_j, start1, end1, start2, end2);
    // Se asteapta ca thread-urile sa termine rularea algoritmului Julia
    // inainte de a se apuca de Mandelbrot
    pthread_barrier_wait(&barrier);


    // Recalcularea capetelor intervalului pentru Mandelbrot 
    start1 = thread_id* (double)width_m / P ; 
    end1 = min((thread_id + 1) * (double)width_m / P  - 1, width_m);
    // Recalcularea capetelor intervalului pentru Mandelbrot
    start2 = thread_id* (double)(height_m / 2) / P;;
    end2 = min((thread_id + 1) * (double)(height_m / 2) / P  - 1, (height_m / 2));


    // Se ruleaza algoritmul Mandelbrot
    run_mandelbrot(&par_m, result_m, start1, end1, start2, end2);
    // Se asteapta ca thread-urile sa termine rularea algoritmului Mandelbrot
    pthread_barrier_wait(&barrier);


	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	int i, r;
	pthread_t threads[P];
	int thread_id[P];

	// se citesc argumentele programului
	get_args(argc, argv);

	// Julia:
	// - se citesc parametrii de intrare
	// - se aloca tabloul cu rezultatul
	// - se ruleaza algoritmul
	// - se scrie rezultatul in fisierul de iesire
	// - se elibereaza memoria alocata

		// Mandelbrot:
	// - se citesc parametrii de intrare
	// - se aloca tabloul cu rezultatul
	// - se ruleaza algoritmul
	// - se scrie rezultatul in fisierul de iesire
	// - se elibereaza memoria alocata

	// Julia:
	// - se citesc parametrii de intrare
	// - se aloca tabloul cu rezultatul
	read_input_file(in_filename_julia, &par_j);

	width_j = (par_j.x_max - par_j.x_min) / par_j.resolution;
	height_j = (par_j.y_max - par_j.y_min) / par_j.resolution;
	result_j = allocate_memory(width_j, height_j);


	// Mandelbrot:
	// - se citesc parametrii de intrare
	// - se aloca tabloul cu rezultatul
	read_input_file(in_filename_mandelbrot, &par_m);

	width_m = (par_m.x_max - par_m.x_min) / par_m.resolution;
	height_m = (par_m.y_max - par_m.y_min) / par_m.resolution;
	result_m = allocate_memory(width_m, height_m);


	// Se creeaza bariera pentru P thread-uri
	pthread_barrier_init(&barrier, NULL, P);
	if (r) {
		printf("Eroare la crearea barierei");
		exit(-1);
	}

	// Se creeaza thread-urile care ruleaza algoritmii
	for (i = 0; i < P; i++) {
		thread_id[i] = i;
		pthread_create(&threads[i], NULL, thread_function, &thread_id[i]);
	}

	// Se asteapta thread-urile
	for (i = 0; i < P; i++) {
		pthread_join(threads[i], NULL);
	}

	// Distrugere bariera
	pthread_barrier_destroy(&barrier);


	// Julia:
	// - se scrie rezultatul in fisierul de iesire
	// - se elibereaza memoria alocata
	write_output_file(out_filename_julia, result_j, width_j, height_j);
	free_memory(result_j, height_j);

	// Mandelbrot:
	// - se scrie rezultatul in fisierul de iesire
	// - se elibereaza memoria alocata
	write_output_file(out_filename_mandelbrot, result_m, width_m, height_m);
	free_memory(result_m, height_m);

	return 0;
}

