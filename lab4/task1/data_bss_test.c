// test1
// int data_var = 42; // .data
// int bss_var; // .bss

//test2
// массив в .data 
// int arr1[1000] = {1};  // .data (не нули)
// массив в .bss (неинициализирован)
// int arr2[1000];

// test3
// массив в .bss
int arr1[1000] = {0};  // .bss (да инициализирован, но нулями -> .bss)
int arr2[1000]; // .bss


int main() { 
    return 0; 
}