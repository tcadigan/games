#ifndef MOVE_H_
#define MOVE_H_

void arm(int, int, int, int);
void move_popn(int, int, int);
void walk(int, int, int);
int get_move(char, int, int, int *, int *, planettype *);
void mech_defend(int, int, int *, int, planettype *, int, int,
                            sectortype *, int, int, sectortype *);
void mech_attac_people(shiptype *, int *, int *, racetype *,
                                  racetype *, sectortype *, int, int, int,
                                  char *, char *);
void people_attack_mech(shiptype *, int, int, racetype *, racetype *,
                                   sectortype *, int, int, char *, char *);
void ground_attack(racetype *, racetype *, int *, int,
                              unsigned short *, unsigned shot *, unsigned int,
                              unsigned int, double, double, double *, double *,
                              int *, int *, int *);

#endif /* MOVE_H_ */
