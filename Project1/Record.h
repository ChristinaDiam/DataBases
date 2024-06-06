#ifndef _RECORD_H_
#define _RECORD_H_

#define NAME_SIZE      15
#define SURNAME_SIZE   25
#define ADDRESS_SIZE   50

/* We define the 'Record' structure. The records we will
 * be saving in the disk will have the following form
 */

typedef struct
{
    int id;
    char name[NAME_SIZE];
    char surname[SURNAME_SIZE];
    char address[ADDRESS_SIZE];

} Record;

#endif /* _RECORD_H_ */
