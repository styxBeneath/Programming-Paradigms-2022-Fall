#include "hashset.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void HashSetNew(hashset *h, int elemSize, int numBuckets,
		HashSetHashFunction hashfn, HashSetCompareFunction comparefn, HashSetFreeFunction freefn) {
	assert(elemSize > 0 && numBuckets > 0 && hashfn != NULL && comparefn != NULL);
	h -> elem_size = elemSize;
	h -> num_buckets = numBuckets;
	h -> hash_fn = hashfn;
	h -> comp_fn = comparefn;
	h -> free_fn = freefn;
	h -> log_len = 0;
	h ->buckets = malloc(sizeof(vector*) * numBuckets);
	for(int i=0; i<numBuckets; i++) {
		h -> buckets[i] = malloc(sizeof(vector));
		VectorNew(h -> buckets[i], elemSize, freefn, 10);
	}
}

void HashSetDispose(hashset *h) {
	for (int i=0; i<h -> num_buckets; i++) {
		VectorDispose(h -> buckets[i]);
		free(h -> buckets[i]);
	}
	free(h -> buckets);
}

int HashSetCount(const hashset *h){ 
	return h -> log_len; 
}

void HashSetMap(hashset *h, HashSetMapFunction mapfn, void *auxData) {
	assert(mapfn != NULL);
	for(int i=0; i< h -> num_buckets; i++) {
		for(int j=0; j<VectorLength(h -> buckets[i]); j++) {
			mapfn(VectorNth(h -> buckets[i], j), auxData);
		}
	}
}

void HashSetEnter(hashset *h, const void *elemAddr) {
	int index = h -> hash_fn(elemAddr, h -> num_buckets);
	assert(index >= 0 && index < h -> num_buckets);
	int position = VectorSearch(h -> buckets[index], elemAddr, h -> comp_fn, 0, false);
	if(position == -1) {
		VectorAppend(h -> buckets[index], elemAddr);
	} else {
		VectorReplace(h -> buckets[index], elemAddr, position);
	}
	h -> log_len++;
}

void *HashSetLookup(const hashset *h, const void *elemAddr) { 
	int index = h -> hash_fn(elemAddr, h -> num_buckets);
	vector* vec = h -> buckets[index];
	int position = VectorSearch(vec, elemAddr, h -> comp_fn, 0, false);
	if(position == -1) return NULL;
	return VectorNth(vec, position);
}
