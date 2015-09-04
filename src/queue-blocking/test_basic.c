/*
 *  simple single-threaded test
 */
#include "bl-queue.h"

int main() {
    queue_t* queue = create_queue(1);
    qvalue_t* op;

    print_queue(queue);

    op = (qvalue_t*) malloc(sizeof(qvalue_t));
    *op = 1;
    enqueue(queue, op, 0);
    print_queue(queue);

    op = (qvalue_t*) malloc(sizeof(qvalue_t));
    *op = 2;
    enqueue(queue, op, 0);
    print_queue(queue);

    op = (qvalue_t*) malloc(sizeof(qvalue_t));
    *op = 3;
    enqueue(queue, op, 0);
    print_queue(queue);

    op = dequeue(queue, 0);
    fprintf(stderr, "dequeueueued value is %llu\n",*op);
    print_queue(queue);

    op = dequeue(queue, 0);
    fprintf(stderr, "dequeueueued value is %llu\n",*op);
    print_queue(queue);

    op = dequeue(queue, 0);
    fprintf(stderr, "dequeueueued value is %llu\n",*op);
    print_queue(queue);

    op = (qvalue_t*) malloc(sizeof(qvalue_t));
    *op = 4;
    enqueue(queue, op, 0);
    print_queue(queue);

    return 0;
}
