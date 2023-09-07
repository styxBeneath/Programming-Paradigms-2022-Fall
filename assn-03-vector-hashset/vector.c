#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void VectorNew(vector *v, int elemSize, VectorFreeFunction freeFn, int initialAllocation){
    v -> elem_size = elemSize;
    v -> alloc_len = initialAllocation == 0 ? 10 : initialAllocation;
    v -> log_len = 0;
    v -> free_function = freeFn;
    v -> base = malloc(v -> alloc_len * v -> elem_size);
    assert(v -> base != NULL);
}

void VectorDispose(vector *v){
    if(v -> free_function != NULL) {
        for(int i=0; i<v -> log_len; i++) {
            v -> free_function((char*) v -> base + i * v -> elem_size);
        }
    }
    free(v -> base);
}

int VectorLength(const vector *v){
    return v -> log_len; 
}

void *VectorNth(const vector *v, int position){
    return (char*)v -> base + position * v -> elem_size;
}

void VectorReplace(vector *v, const void *elemAddr, int position){
    assert(position >= 0 && position < v -> log_len);
    if(v -> free_function != NULL){
        v -> free_function((char*)v -> base + position * v -> elem_size);
    }
    memcpy((char*)v -> base + position * v -> elem_size, elemAddr, v -> elem_size);
}

void VectorInsert(vector *v, const void *elemAddr, int position){
    if(v -> log_len == v -> alloc_len) {
        v -> alloc_len = v -> alloc_len * 2;
        v -> base = realloc(v -> base, v -> alloc_len * v -> elem_size);
        assert(v -> base != NULL);
    }
    assert(position >= 0 && position <= v -> log_len);
    if(position < v -> log_len) {
        memmove(VectorNth(v, position + 1), VectorNth(v, position), (v -> log_len - position) * v -> elem_size);
    }
    memmove(VectorNth(v, position), elemAddr, v -> elem_size);
    v -> log_len++;
}

void VectorAppend(vector *v, const void *elemAddr){
    VectorInsert(v, elemAddr, v -> log_len);
}

void VectorDelete(vector *v, int position){
    assert(position >= 0 && position < v -> log_len);
    if(v -> free_function != NULL) {
        v -> free_function((char*) v -> base + position * v -> elem_size);
    }
    memmove(VectorNth(v, position), VectorNth(v, position+1), (v -> log_len - position - 1) * v -> elem_size);
    v -> log_len--;
}

void VectorSort(vector *v, VectorCompareFunction compare){
    qsort(v -> base, v -> log_len, v -> elem_size, compare);
}

void VectorMap(vector *v, VectorMapFunction mapFn, void *auxData){
    assert(mapFn != NULL);
    for(int i=0; i<v -> log_len; i++) {
        mapFn(VectorNth(v, i), auxData);
    }
}

static const int kNotFound = -1;
int VectorSearch(const vector *v, const void *key, VectorCompareFunction searchFn, int startIndex, bool isSorted){ 
    if(isSorted) {
        int end = v -> log_len - 1;
        int start = startIndex;
        while(start <= end) {
            int mid = (start + end)/2;
            if(searchFn(VectorNth(v,mid), key) == 0) {
                return mid;
            }
            if(searchFn(VectorNth(v,mid), key) > 0) {
                end = mid - 1;
            } else {
                start = mid + 1;
            }
        }
        return kNotFound;
    } else {
        for (int i = startIndex; i < v -> log_len; i++){
            if (searchFn(VectorNth(v, i), key) == 0){
                return i;
            } 
        }
        return kNotFound;
    }
} 
