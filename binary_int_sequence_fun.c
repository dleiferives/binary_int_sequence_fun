// (c) Dylan Leifer-Ives 2024
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdint.h>

/// Type definitions
/// u{8,16,32,64,128} - unsigned integer types
/// s{8,16,32,64,128} - signed integer types
/// f{32,64} - floating point types
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float f32;
typedef double f64;

#if defined(__SIZEOF_INT128__)
typedef __uint128_t u128;
typedef __int128_t s128;
#endif // __SIZEOF_INT128__

// List implementation
typedef struct List {
	u64 *data;
	u64 size;
	u64 capacity;
} List;

List *list_new(u64 capacity) {
	List *list = malloc(sizeof(List));
	list->data = malloc(capacity * sizeof(u64));
	list->size = 0;
	list->capacity = capacity;
	return list;
}

// resize to double the current capacity
void list_resize(List *list) {
	list->capacity *= 2;
	list->data = realloc(list->data, list->capacity * sizeof(u64));
}

// free the list and its data
void list_free(List **list) {
	free((*list)->data);
	(*list)->data = NULL;
	free(*list);
	*list = NULL;
}

// add an element to the end of the list
void list_push(List *list, u64 value) {
	if (list->size == list->capacity) {
		list_resize(list);
	}
	list->data[list->size++] = value;
}

// add an element to the beginning of the list
void list_unshift(List *list, u64 value) {
	if (list->size == list->capacity) {
		list_resize(list);
	}
	for (u64 i = list->size; i > 0; i--) {
		list->data[i] = list->data[i - 1];
	}
	list->data[0] = value;
	list->size++;
}
typedef struct{
	u8 *data;
	u64 size;
	u64 capacity;
} DecString;

DecString *dec_string_new(u64 capacity) {
	DecString *dec_string = malloc(sizeof(DecString));
	dec_string->data = malloc(capacity * sizeof(u8));
	dec_string->size = 0;
	dec_string->capacity = capacity;
	return dec_string;
}

void dec_string_resize(DecString *dec_string) {
	dec_string->capacity *= 2;
	dec_string->data = realloc(dec_string->data, dec_string->capacity * sizeof(u8));
}

DecString *dec_string_free(DecString *dec_string) {
	free(dec_string->data);
	dec_string->data = NULL;
	return NULL;
}

void dec_string_push(DecString *dec_string, u8 value) {
	if (dec_string->size == dec_string->capacity) {
		dec_string_resize(dec_string);
	}
	dec_string->data[dec_string->size++] = value;
}

void dec_string_unshift(DecString *dec_string, u8 value) {
	if (dec_string->size == dec_string->capacity) {
		dec_string_resize(dec_string);
	}
	for (u64 i = dec_string->size; i > 0; i--) {
		dec_string->data[i] = dec_string->data[i - 1];
	}
	dec_string->data[0] = value;
	dec_string->size++;
}

DecString dec_string_zero(void) {
	DecString dec_string = {
		.capacity = 100,
		.size = 0,
		.data = malloc(100 * sizeof(u8)),
	};
	memset(dec_string.data, 0, 100);
	return dec_string;
}

void dec_string_double(DecString *dec_string) {
	u8 carry = 0;
	for (u64 i = 0; i < dec_string->size; i++) {
		u8 temp = (dec_string->data[i] * 2) + carry;
		dec_string->data[i] = temp % 10;
		carry = temp / 10;
	}
	if (carry) {
		dec_string_push(dec_string, carry);
	}
}

void dec_string_add_into(DecString *dec_string, DecString *other) {
	u8 carry = 0;
	while(other->size > dec_string->capacity){
		dec_string_resize(dec_string);
	}
	for (u64 i = 0; i < other->size; i++) {
		u8 temp = dec_string->data[i] + other->data[i] + carry;
		dec_string->data[i] = temp % 10;
		carry = temp / 10;
	}
	for (u64 i = other->size; i < dec_string->size; i++) {
		u8 temp = dec_string->data[i] + carry;
		dec_string->data[i] = temp % 10;
		carry = temp / 10;
	}
	if (carry) {
		dec_string_push(dec_string, carry);
	}
}

void dec_string_print(DecString *dec_string) {
	for (u64 i = dec_string->size; i > 0; i--) {
		printf("%d", dec_string->data[i - 1]);
	}
	printf("\n");
}


////////// Big Integer implementation///////////////////////////////////////////

typedef struct {
	List *data;
	u8 sign;
} BigInt;


BigInt *big_int_new() {
	BigInt *big_int = malloc(sizeof(BigInt));
	big_int->data = list_new(1);
	big_int->sign = 0;
	return big_int;
}

////////////////////////////////////////////////////////////////////////////////

u64 find_sequence_count(u8 *data, u64 dlen, u64 seq_start, u64 seq_len){
	u64 count = 0;
	for(u64 i = seq_start; i < dlen; i+=seq_len){
		u8 valid = memcmp(&data[seq_start], &data[i], seq_len);
		if(valid == 0){
			count++;
		} else {
			return count;
		}
	}
	return count;
}


u8 find_matching_sequence(u64 n, u8 *seq, u64 len, u64 *res_start, u64 *res_len){
		u64 seq_len = 4;
		for(u32 ni = 0; ni < n; ni++){
			seq_len *= 5;
		}
		u64 seq_start = n+1;
		s64 seq_count = find_sequence_count(seq, len, seq_start, seq_len);
		if(((seq_count) * (seq_len)) >= len - seq_len){ // have to count for the fact that the sequence is not included in the count
			*res_start = seq_start;
			*res_len = seq_len;
			return 1;
		} 
	return 0;
}

DecString find_nth_sequence(u64 n){
	DecString result = dec_string_zero();
	DecString nth_item = dec_string_zero();
	DecString dec_string = dec_string_zero();
	while(dec_string.capacity < n){
		dec_string_resize(&dec_string);
	}

	dec_string_push(&dec_string, 1);
	for(u64 i = 0; i < 100; i++){
		dec_string_push(&nth_item, dec_string.data[n]);
		dec_string_double(&dec_string);
	}

	uint64_t start = 0;
	uint64_t len = 0;
	u8 found = find_matching_sequence(n, nth_item.data, nth_item.size, &start, &len);
	u64 iterations = 0;
	while(!found){
		iterations++;	
		if(iterations > 1000){
			printf("Failed to find a matching sequence\n");
			exit(1);
		}
		for(u64 i = 0; i < 1000; i++){
			dec_string_push(&nth_item, dec_string.data[n]);
			dec_string_double(&dec_string);
		}
		found = find_matching_sequence(n, nth_item.data, nth_item.size, &start, &len);
	}

	for(u64 i = 0; i < len; i++){
		dec_string_push(&result, nth_item.data[start + i]);
	}

	printf("Found sequence:\n");
	dec_string_print(&result);
	// printf("\nInside find_nth_sequence:\n");
	// dec_string_print(&nth_item);
	printf("with length %d\n", len);

	dec_string_free(&nth_item);
	dec_string_free(&dec_string);

	return result;
}


s32 main(int argc, char **argv){
	u64 n =2;
	if(argc > 1){
		n = atoi(argv[1]);
	}
	DecString result = find_nth_sequence(n);
	// dec_string_print(&result);
	return 0;
}








