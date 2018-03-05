#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef enum {
	FIFO,
	LRU
}PageReplacementAlgorithms;

typedef struct {
	int id;
	char hitOrPageFault[3];
}PageRequest;

PageRequest* NewPageRequest(const int id);

typedef struct Node {
	PageRequest* item;
	struct Node* next;
}Node;

Node* NewNode(PageRequest* const item);

typedef struct {
	int id;
	int timesRequested;
}SimNode;

typedef struct {
	Node* head;
	Node* tail;
}Queue;

Queue* NewQueue();
void Enqueue(Queue* const queue, Node* const item);
Node* Dequeue(Queue* const queue);
bool IsQueueEmpty(const Queue* const queue);
bool IsSimFull(SimNode* const head, const int size);
bool SimContains(SimNode* const head, const int size, const int id);
void SortByTimesRequested(SimNode* const head, const int size);
void FreeQueue(Queue* const queueToFree);

bool CheckAndInitialiseInput(int argc, char* argv[], PageReplacementAlgorithms* const algorithmToCall, int* const totalFrames);
Queue* ReadFileData(char* inputFileName);

void SimulateFIFO(Queue* const inputQueue, const int totalFrames, char* outputFileName);
void SimulateLRU(Queue* const inputQueue, const int totalFrames, char* outputFileName);

void PrintValues(Queue* const outputQueue, char* outputFileName);

int main(int argc, char* argv[]) {
	PageReplacementAlgorithms algorithmToCall;
	int totalFrames;
	char inputFileName[50];
	char outputFileName[50];
	if (!CheckAndInitialiseInput(argc, argv, &algorithmToCall, &totalFrames)) {
		printf("Exiting...");
		exit(0);
	}
	strcpy(inputFileName, argv[3]);
	strcpy(outputFileName, argv[4]);
	Queue* inputQueue = ReadFileData(inputFileName);
	if (inputQueue == NULL) {
		printf("Exiting...");
		exit(0);
	}
	switch (algorithmToCall) {
	case FIFO:
		SimulateFIFO(inputQueue, totalFrames, outputFileName);
		break;
	case LRU:
		SimulateLRU(inputQueue, totalFrames, outputFileName);
		break;
	}
	free(inputQueue);
	return 0;
}

PageRequest* NewPageRequest(const int id){
	PageRequest* temp = (PageRequest*)malloc(sizeof(PageRequest));
	if (temp == NULL) {
		printf("Error! System ran out of memory!\n");
		return NULL;
	}
	temp->id = id;
	return temp;
}

Node* NewNode(PageRequest* const item) {
	Node* temp = (Node*)malloc(sizeof(Node));
	if (temp == NULL) {
		printf("Error! System ran out of memory!\n");
		return NULL;
	}
	temp->item = item;
	temp->next = NULL;
	return temp;
}

Queue* NewQueue() {
	Queue* temp = (Queue*)malloc(sizeof(Queue));
	if (temp == NULL) {
		printf("Error! System ran out of memory!");
		return NULL;
	}
	temp->head = temp->tail = NULL;
	return temp;
}

void Enqueue(Queue* const queue, Node* const item) {
	if (queue->tail == NULL) {
		queue->head = queue->tail = item;
	}
	else {
		queue->tail->next = item;
		queue->tail = item;
	}
}

Node* Dequeue(Queue* const queue) {
	if (queue->head == NULL) {
		return NULL;
	}
	Node* temp = queue->head;
	queue->head = queue->head->next;
	temp->next = NULL;
	return temp;
}

bool IsQueueEmpty(const Queue* const queue) {
	return (queue->head == NULL);
}

bool IsSimFull(SimNode* const head, const int size) {
	for (int i = 0; i < size; i++) {
		if ((head+i)->id == -1) {
			return false;
		}
	}
	return true;
}

bool SimContains(SimNode* const head, const int size, const int id) {
	for (int i = 0; i < size; i++) {
		if ((head + i)->id == id) {
			return true;
		}
	}
	return false;
}

void SortByTimesRequested(SimNode* const head, const int size) {
	int minTimesRequested = head->timesRequested;
	int minPos = 0;
	for (int i = 1; i < size; i++) {
		if ((head + i)->timesRequested < minTimesRequested && (head + i)->id != -1) {
			minTimesRequested = (head + i)->timesRequested;
			minPos = i;
		}
	}
	if (minPos != 0) {
		int tempID = head->id;
		int tempTimesRequested = head->timesRequested;
		head->id = (head + minPos)->id;
		head->timesRequested = (head + minPos)->timesRequested;
		(head + minPos)->id = tempID;
		(head + minPos)->timesRequested = tempTimesRequested;
	}
}

void FreeQueue(Queue* const queueToFree) {
	Node* iterator = queueToFree->head;
	Node* deleter;
	while (iterator != NULL) {
		deleter = iterator;
		iterator = iterator->next;
		free(deleter->item);
		free(deleter);
	}
}

bool CheckAndInitialiseInput(int argc, char* argv[], PageReplacementAlgorithms* const algorithmToCall, int* const totalFrames) {
	if (argc != 5) {
		printf("Error! Invalid number of command arguments! \n");
		return false;
	}
	for (int i = 0; argv[1][i]; i++) {
		argv[1][i] = tolower(argv[1][i]);
	}
	*totalFrames = atoi(argv[2]);
	if (*totalFrames < 1) {
		printf("Error! Memory frames can't be less than 1\n");
		return false;
	}
	if (strcmp(argv[1], "fifo") == 0) {
		*algorithmToCall = FIFO;
		return true;
	}
	else if (strcmp(argv[1], "lru") == 0) {
		*algorithmToCall = LRU;
		return true;
	}
	else {
		printf("Error! Invalid algorithm call please use 'LRU' or 'FIFO'! \n");
		return false;
	}
}

Queue* ReadFileData(char* inputFileName) {
	FILE* inputFile = fopen(inputFileName, "r");
	if (inputFile == NULL) {
		printf("Error! File %s could not be opened!\n", inputFileName);
		return NULL;
	}
	int pageID;
	Queue* inputQueue = NewQueue();
	if (inputQueue == NULL) {
		return NULL;
	}
	while (fscanf(inputFile, "%d", &pageID) == 1) {
		PageRequest* newPage = NewPageRequest(pageID);
		if (newPage == NULL) {
			return NULL;
		}
		Node* newNode = NewNode(newPage);
		if (newNode == NULL) {
			return NULL;
		}
		Enqueue(inputQueue, newNode);
	}
	fclose(inputFile);
	return inputQueue;
}

void SimulateFIFO(Queue* const inputQueue, const int totalFrames, char* outputFileName) {
	SimNode* simulationQueue = (SimNode*)malloc(totalFrames * sizeof(SimNode));
	if (simulationQueue == NULL) {
		printf("Error! System ran out of memory! \n");
		printf("Exiting...");
		exit(0);
	}
	for (int i = 0; i < totalFrames; i++) {
		(simulationQueue + i)->id = -1;
	}
	Queue* outputQueue = NewQueue();
	while (!IsQueueEmpty(inputQueue)) {
		Node* temp = Dequeue(inputQueue);
		if (SimContains(simulationQueue, totalFrames, temp->item->id)) {
			strcpy(temp->item->hitOrPageFault, "HIT");
			Enqueue(outputQueue, temp);
		}
		else {
			strcpy(temp->item->hitOrPageFault, "PF");
			Enqueue(outputQueue, temp);
			if (IsSimFull(simulationQueue, totalFrames)) {
				int tempID = simulationQueue->id;
				for (int i = 0; i < totalFrames - 1; i++) {
					(simulationQueue + i)->id = (simulationQueue + i + 1)->id;
					(simulationQueue + i + 1)->id = tempID;
				}
				(simulationQueue + totalFrames - 1)->id = temp->item->id;
			}
			else {
				for (int i = 0; i < totalFrames; i++) {
					if ((simulationQueue + i)->id == -1) {
						(simulationQueue + i)->id = temp->item->id;
						break;
					}
				}
			}
		}
	}
	PrintValues(outputQueue, outputFileName);
	FreeQueue(outputQueue);
	free(outputQueue);
	free(simulationQueue);
}

void SimulateLRU(Queue* const inputQueue, const int totalFrames, char* outputFileName) {
	SimNode* simulationQueue = (SimNode*)malloc(totalFrames * sizeof(SimNode));
	if (simulationQueue == NULL) {
		printf("Error! System ran out of memory! \n");
		printf("Exiting...");
		exit(0);
	}
	for (int i = 0; i < totalFrames; i++) {
		(simulationQueue + i)->id = -1;
		(simulationQueue + i)->timesRequested = 0;
	}
	Queue* outputQueue = NewQueue();
	while (!IsQueueEmpty(inputQueue)) {
		Node* temp = Dequeue(inputQueue);
		if (SimContains(simulationQueue, totalFrames, temp->item->id)) {
			strcpy(temp->item->hitOrPageFault, "HIT");
			Enqueue(outputQueue, temp);
			for (int i = 0; i < totalFrames; i++) {
				if ((simulationQueue + i)->id == temp->item->id) {
					(simulationQueue + i)->timesRequested++;
					break;
				}
			}
		}
		else {
			strcpy(temp->item->hitOrPageFault, "PF");
			Enqueue(outputQueue, temp);
			if (IsSimFull(simulationQueue, totalFrames)) {
				simulationQueue->id = temp->item->id;
				simulationQueue->timesRequested = 0;
			}
			else {
				for (int i = 0; i < totalFrames; i++) {
					if ((simulationQueue + i)->id == -1) {
						(simulationQueue + i)->id = temp->item->id;
						break;
					}
				}
			}
		}
		SortByTimesRequested(simulationQueue, totalFrames);
	}
	PrintValues(outputQueue, outputFileName);
	FreeQueue(outputQueue);
	free(outputQueue);
	free(simulationQueue);
}

void PrintValues(Queue* const outputQueue, char* outputFileName) {
	FILE* outputFile = fopen(outputFileName, "w");
	if (outputFile == NULL) {
		printf("Error! File %s could not be opened! \n", outputFileName);
		printf("Exiting..");
		exit(0);
	}
	int numOfPageFaults = 0;
	int numOfPages = 0;
	for (Node* iterator = outputQueue->head; iterator != NULL; iterator = iterator->next) {
		numOfPages++;
		if (strcmp(iterator->item->hitOrPageFault, "PF") == 0) {
			numOfPageFaults++;
		}
		fprintf(outputFile, "%d\t%s\n", iterator->item->id, iterator->item->hitOrPageFault);
	}
	float pfRate = (float)numOfPageFaults / numOfPages * 100;
	fprintf(outputFile, "PF Rate : %.2f%%", pfRate);
	fclose(outputFile);
}
