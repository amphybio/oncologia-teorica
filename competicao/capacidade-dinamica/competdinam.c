/* Licença:

*/

/*
   Este é o arquivo fonte do código para o cálculo do crescimento tumoral
considerando que a capacidade do sistema também depende do tempo.

   Última modificação: AFR 29/08/2019.
*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "sys/time.h"
#include <unistd.h> /* for get opt */

/*************** constants ********************************/

#define OPTS ":suh:" /* for the output type */

static const char usage[] =
"\t Usage: ./odyn [options] <input file name>.in <output file name>.out <log file name>.log\n\n"

  "\t Options:\n";

long double coeff_f1(long double lambda, long double xi, long double x, long double K)
{
  long double f1;
  long double aux = log(x/K);
  f1 = -lambda*x*aux - xi*x;
  //  printf("lambda=%Lf, x=%Lf, K=%Lf, log(x/K)=%Lf, f1=%Lf\n",lambda,x,K,aux,f1);
  return f1;
}

long double trat(long double t, long double per, long double c)
{
  long double ti, g;
  ti=0.;
  g=0.;

  while (ti <= t){
    g+=exp(-c*(t-ti));
    ti += per;
  }

  return g;
}

long double coeff_f2(long double phi, long double psi, long double nu, long double per, long double c, long double dose, long double t, long double x, long double K)
{
  long double XPNT=0.6666666666666667;

  long double f2, bux;
  bux = trat(t, per, c);
  //  printf("this is bux=%Lf\n",bux);
  bux = dose*bux;
  //printf("this is bux=%Lf\n",bux);
  f2 = phi*x-psi*K*pow(x,XPNT) - nu*K*bux;
  return f2;
}

int main(int argc, char * * argv) {

  int cc; // parses command line options
  //printf("%s",usage);
  //printf("%s",help);

  optarg = NULL;

  while ( ( cc=getopt(argc,argv,OPTS))!=-1){
    switch (cc) {
    case 'u':
      printf("%s",usage);
      break;
    }
  }
  //printf("This is the argc=%d\n",argc);
  //printf("This is the argv[%d]=%s\n",argc,argv[argc]);

  /* Aqui declaramos as variáveis usadas nesse código */

  int i, j;	//inteiros para contagem nos laços

  // --- eq. 1: on x ---/
  long double lambda, x, xi;
  /*
    lambda: taxa de crescimento das células tumorais
    x: densidade de células tumorais
    xi: capacidade de degradação da célula tumoral do tratamento
    empregado, com 0 <= xi <= 1 (quimioterapia ou imunoterapia).
   */

  // --- eq. 2: on K ---/
  long double K, phi, psi, nu, D, M, c, Ti;
  /*
    K: fator de capacidade do tumor psi: constante de crescimento da capacidade
    phi: constante de decaimento da capacidade g(t): concentração da droga no
    instante t nu: taxa de efetividade do tratamento (pode ser incorporada em
    g(t), mas não o faremos por motivações pedagógicas.)

    Assumindo um tratamento periódico, em que as quantidades da droga decaem exponencialmente, temos
    c: taxa de decaimento da quantidade de droga ainda efetiva
    e, para:

    a(t): a função de ministração da droga, escrevemos:

    g(t) = int_0^t {a(tl) exp(-c(t-tl) dtl)}

    e, se o tratamento ocorrer periodicamente, denominaremos por

    Ti: intervalo entre dois eventos de tratamento
    D: dose da droga a cada ocorrência de tratamento.

    assim, podemos definir a(t) como:

    a(t) = sum_{i=0}^{n-1} D delta (t-ti),

    em que

    ti = i*M
    e
    n: numero de ocorrências do tratamento.

   */

  // --- Numerical solution parameters ---//
  long double h,t;
  // h: passo de integração; t: tempo em unidades arbitrárias.
  unsigned long long int n;
  // número de passos de integração

  // ********************************************************* //

  /* Aqui introduzimos as funções de leitura de variáveis
     nos arquivos de entrada, e os arquivos log e de saída do
     programa.
  */

  char infile[80], outfile[80];		//files
  char logfile[80], tmp[80];
  FILE * fin, * fout, * flog;
  struct timeval start, end;
  float all_time;

  if (argc < 4) {
    printf("Please add 3 parameter: inputfile, outputfile, logfile\n");
    exit(0);
  }

  strcpy(infile, argv[argc-3]);
  strcpy(outfile, argv[argc-2]);
  strcpy(logfile, argv[argc-1]);

  strcpy(tmp, "date ");
  strncat(tmp, ">> ", 3);
  strncat(tmp, logfile, strlen(logfile));
  flog = fopen(logfile, "w");
  fprintf(flog, "------------Start time------------\n");
  fclose(flog);
  if(system(tmp)) printf("Ok");

  gettimeofday( & start, NULL);

  /* -------------------------------------------------------------------*/

  fin = fopen(infile, "r");
  if (fin == NULL) {
    printf("File %s is  not exist\n", infile);
    exit(0);
  }
  if ((fscanf(fin, "Parametros\n"))) printf("Error: Parametros\n");
  if ((fscanf(fin, "#Eq1\n"))) printf("Error: Eq1\n");
  if (!(fscanf(fin, "lambda=%Lf\n", &lambda ))) printf("Error: variable lambda\n");
  if (!(fscanf(fin, "xi=%Lf\n", &xi ))) printf("Error: variable xi\n");
  if ((fscanf(fin, "##\n"))) printf("Error: ##\n");

  if ((fscanf(fin, "#Eq2\n"))) printf("Error: Eq2\n");
  if (!(fscanf(fin, "phi=%Lf\n", &phi ))) printf("Error: variable psi\n");
  if (!(fscanf(fin, "psi=%Lf\n", &psi ))) printf("Error: variable phi\n");
  if (!(fscanf(fin, "nu=%Lf\n", &nu ))) printf("Error: variable nu\n");
  if (!(fscanf(fin, "D=%Lf\n", &D ))) printf("Error: variable D\n");
  if (!(fscanf(fin, "c=%Lf\n", &c ))) printf("Error: variable c\n");
  if (!(fscanf(fin, "Ti=%Lf\n", &Ti ))) printf("Error: variable Ti\n");
  if ((fscanf(fin, "##\n"))) printf("Error: ##\n");

  if ((fscanf(fin, "==>IntlCndtns\n"))) printf("Error: IntlCndtns\n");
  if (!(fscanf(fin, "x=%Lf\n", &x ))) printf("Error: variable x\n");
  if (!(fscanf(fin, "K=%Lf\n", &K ))) printf("Error: variable K\n");

  if ((fscanf(fin, "==>NumSimParms\n"))) printf("Error: NumSimParms\n");
  if (!(fscanf(fin, "h=%Lf\n", &h ))) printf("Error: variable h\n");
  if (!(fscanf(fin, "t=%Lf\n", &t ))) printf("Error: variable t\n");
  if (!(fscanf(fin, "n=%llu\n", &n ))) printf("Error: variable n\n");

  fclose(fin);

  printf("==>Parametros\n");
  printf("#Eq1\n");
  printf("lambda=%Lf\n",lambda);
  printf("xi=%Lf\n",xi);
  printf("##\n");
  printf("#Eq2\n");
  printf("phi=%Lf\n",phi);
  printf("psi=%Lf\n",psi);
  printf("nu=%Lf\n",nu);
  printf("D=%Lf\n",D);
  printf("c=%Lf\n",c);
  printf("Ti=%Lf\n",Ti);
  printf("##\n");

  printf("==>IntlCndtns\n");
  printf("x=%Lf\n",x);
  printf("K=%Lf\n",K);

  printf("==>NumSimParms\n");
  printf("h=%Lf\n",h);
  printf("t=%Lf\n",t);
  printf("n=%llu\n",n);

  flog = fopen(logfile, "a");

  fprintf(flog, "==>Parametros\n");
  fprintf(flog, "#Eq1\n");
  fprintf(flog, "lambda=%Lf\n",lambda);
  fprintf(flog, "xi=%Lf\n",xi);
  fprintf(flog, "##\n");
  fprintf(flog, "#Eq2\n");
  fprintf(flog, "phi=%Lf\n",phi);
  fprintf(flog, "psi=%Lf\n",psi);
  fprintf(flog, "nu=%Lf\n",nu);
  fprintf(flog, "D=%Lf\n",D);
  fprintf(flog, "c=%Lf\n",c);
  fprintf(flog, "Ti=%Lf\n",Ti);
  fprintf(flog, "##\n");
  fprintf(flog, "==>IntlCndtns\n");
  fprintf(flog, "x=%Lf\n",x);
  fprintf(flog, "K=%Lf\n",K);
  fprintf(flog, "==>NumSimParms\n");
  fprintf(flog, "h=%Lf\n",h);
  fprintf(flog, "t=%Lf\n",t);
  fprintf(flog, "n=%llu\n",n);

  fclose(flog);

  fout=fopen(outfile,"w");

  coeff_f1(lambda, xi, x, K);
  coeff_f2(phi, psi, nu, Ti, c, D, t, x, K);

  fprintf(fout,"%Lf \t %Lf \t %Lf\n",t,x,K);
  printf("t=%Lf \t x=%Lf \t K=%Lf\n",t,x,K);

  for (i=0;i<n;i++){

    long double A11 = coeff_f1(lambda, xi, x, K);
    long double A12 = coeff_f2(phi, psi, nu, Ti, c, D, t, x, K);

    long double A21 = coeff_f1(lambda, xi, x + h*0.5*A11, K + h*0.5*A12);
    long double A22 = coeff_f2(phi, psi, nu, Ti, c, D, t + 0.5*h, x + h*0.5*A11, K + h*0.5*A12);

    long double A31 = coeff_f1(lambda, xi, x + h*0.5*A21, K + h*0.5*A22);
    long double A32 = coeff_f2(phi, psi, nu, Ti, c, D, t + 0.5*h, x + h*0.5*A21, K + h*0.5*A22);

    long double A41 = coeff_f1(lambda, xi, x + h*A31, K + h*A32);
    long double A42 = coeff_f2(phi, psi, nu, Ti, c, D, t + h, x + h*A31, K + h*A32);

    x += (A11+2.*A21+2.*A31+A41)*h/6.;
    K += (A12+2.*A22+2.*A32+A42)*h/6.;
    t += h;

    fprintf(fout,"%Lf \t %Lf \t %Lf\n",t,x,K);
  }

  fclose(fout);

  gettimeofday( & end, NULL);
  all_time = (end.tv_sec - start.tv_sec) + (float)(end.tv_usec - start.tv_usec) / 1000000.0;
  printf("Run time:\t%f s\n", all_time);
  flog = fopen(logfile, "a");
  fprintf(flog, "\nRun time:\t%f s\n\n", all_time);
  fclose(flog);
  flog = fopen(logfile, "a");
  fprintf(flog, "------------End time------------\n");
  fclose(flog);
  if(system(tmp)) printf("Ok");
  return 1;
}
