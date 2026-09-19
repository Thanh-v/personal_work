#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#define MAX_LEN 32 
#define MAX_TARGETS 10
#define MAX_STATES 128
#define MAX_VARS 8
#define MAX_MINTERMS 256
#define MAX_IMPLICANTS 1024

// =========================================================
// THUẬT TOÁN QUINE-MCCLUSKEY
// =========================================================
typedef struct {
    int val;   
    int mask;  
    bool used; 
} Implicant;

int countBits(int n) {
    int count = 0;
    while (n) { count += n & 1; n >>= 1; }
    return count;
}

bool covers(Implicant imp, int m) {
    return (m & ~imp.mask) == imp.val;
}

void printImplicant(Implicant imp, int vars) {
    bool first = true;
    for (int i = vars - 1; i >= 0; i--) {
        if (!(imp.mask & (1 << i))) { 
            if (!first) printf("");
            if (i == 0) {
                printf((imp.val & (1 << i)) ? "w" : "w'");
            } else {
                printf((imp.val & (1 << i)) ? "Q%d" : "Q%d'", i);
            }
            first = false;
        }
    }
    if (first) printf("1"); 
}

void minimizeQM(char* function_name, int* minterms, int num_m, int* dontcares, int num_d, int vars) {
    if (num_m == 0) {
        printf(">> %s = 0\n", function_name);
        return;
    }

    Implicant implicants[MAX_IMPLICANTS];
    int num_impl = 0;

    for (int i = 0; i < num_m; i++) {
        implicants[num_impl].val = minterms[i];
        implicants[num_impl].mask = 0;
        implicants[num_impl].used = false;
        num_impl++;
    }
    for (int i = 0; i < num_d; i++) {
        implicants[num_impl].val = dontcares[i];
        implicants[num_impl].mask = 0;
        implicants[num_impl].used = false;
        num_impl++;
    }

    Implicant primes[MAX_IMPLICANTS];
    int num_primes = 0;
    bool changed = true;

    while (changed) {
        changed = false;
        Implicant next_impl[MAX_IMPLICANTS];
        int next_count = 0;

        for (int i = 0; i < num_impl; i++) {
            for (int j = i + 1; j < num_impl; j++) {
                if (implicants[i].mask == implicants[j].mask) {
                    int diff = implicants[i].val ^ implicants[j].val;
                    if (countBits(diff) == 1) {
                        implicants[i].used = true;
                        implicants[j].used = true;

                        Implicant new_imp;
                        new_imp.val = implicants[i].val & implicants[j].val; 
                        new_imp.mask = implicants[i].mask | diff;            
                        new_imp.used = false;

                        bool duplicate = false;
                        for (int k = 0; k < next_count; k++) {
                            if (next_impl[k].val == new_imp.val && next_impl[k].mask == new_imp.mask) {
                                duplicate = true; break;
                            }
                        }
                        if (!duplicate) {
                            next_impl[next_count++] = new_imp;
                            changed = true;
                        }
                    }
                }
            }
        }

        for (int i = 0; i < num_impl; i++) {
            if (!implicants[i].used) primes[num_primes++] = implicants[i];
        }

        for (int i = 0; i < next_count; i++) implicants[i] = next_impl[i];
        num_impl = next_count;
    }

    bool m_covered[MAX_MINTERMS] = {false};
    bool p_selected[MAX_IMPLICANTS] = {false};
    int covered_count = 0;

    for (int i = 0; i < num_m; i++) {
        int cover_count = 0;
        int last_p_idx = -1;
        for (int p = 0; p < num_primes; p++) {
            if (covers(primes[p], minterms[i])) {
                cover_count++;
                last_p_idx = p;
            }
        }
        if (cover_count == 1 && !p_selected[last_p_idx]) {
            p_selected[last_p_idx] = true;
            for (int k = 0; k < num_m; k++) {
                if (!m_covered[k] && covers(primes[last_p_idx], minterms[k])) {
                    m_covered[k] = true;
                    covered_count++;
                }
            }
        }
    }

    while (covered_count < num_m) {
        int best_p_idx = -1;
        int max_new_covers = 0;

        for (int p = 0; p < num_primes; p++) {
            if (p_selected[p]) continue;
            int current_covers = 0;
            for (int k = 0; k < num_m; k++) {
                if (!m_covered[k] && covers(primes[p], minterms[k])) current_covers++;
            }
            if (current_covers > max_new_covers) {
                max_new_covers = current_covers;
                best_p_idx = p;
            }
        }

        if (best_p_idx != -1) {
            p_selected[best_p_idx] = true;
            for (int k = 0; k < num_m; k++) {
                if (!m_covered[k] && covers(primes[best_p_idx], minterms[k])) {
                    m_covered[k] = true;
                    covered_count++;
                }
            }
        } else {
            break; 
        }
    }

    printf(">> %s = ", function_name);
    bool first_print = true;
    for (int p = 0; p < num_primes; p++) {
        if (p_selected[p]) {
            if (!first_print) printf(" + ");
            printImplicant(primes[p], vars);
            first_print = false;
        }
    }
    printf("\n");
}

// =========================================================
// QUẢN LÝ DỮ LIỆU ĐA CHUỖI & MÔ HÌNH FSM (MOORE / MEALY)
// =========================================================
char targets[MAX_TARGETS][MAX_LEN];
int num_targets = 0;
int machine_mode = 0; // 0 = Moore, 1 = Mealy

char state_prefixes[MAX_STATES][MAX_LEN]; 
int state_table[MAX_STATES][2];
int output_table[MAX_STATES][2]; // z giờ đây phụ thuộc vào cả ngõ vào w (kích thước 2)
int num_states = 0; 

int findState(const char* str) {
    for (int i = 0; i < num_states; i++) {
        if (strcmp(state_prefixes[i], str) == 0) return i;
    }
    return -1;
}

void addState(const char* str) {
    if (findState(str) == -1) {
        strcpy(state_prefixes[num_states], str);
        num_states++;
    }
}

void generateAllStates() {
    addState(""); 
    
    for (int i = 0; i < num_targets; i++) {
        char temp[MAX_LEN] = "";
        
        // Sự khác biệt 1: Mealy không cần trạng thái chứa toàn bộ chuỗi đích. 
        // Nó chỉ cần tiền tố dài nhất = độ dài chuỗi - 1.
        int max_len = strlen(targets[i]);
        if (machine_mode == 1) {
            max_len--; // Cắt bớt trạng thái cuối cùng cho Mealy
        }

        for (int j = 0; j < max_len; j++) {
            temp[j] = targets[i][j];
            temp[j+1] = '\0';
            addState(temp);
        }
    }
}

int calculateNextState(int current_state, int input_bit) {
    char temp_str[MAX_LEN + 2];
    strcpy(temp_str, state_prefixes[current_state]);
    
    int len = strlen(temp_str);
    temp_str[len] = input_bit + '0';
    temp_str[len + 1] = '\0';
    
    for (int i = 0; i <= strlen(temp_str); i++) {
        char* suffix = temp_str + i; 
        int next_state_id = findState(suffix);
        if (next_state_id != -1) {
            return next_state_id;
        }
    }
    return 0; 
}

// Hàm tính ngõ ra z hợp nhất cho Moore và Mealy
int calculateOutput(const char* state_str, int input_bit) {
    char temp_str[MAX_LEN + 2];
    strcpy(temp_str, state_str);
    
    // Sự khác biệt 2: Tính Output z
    // - Mealy: z đánh giá chuỗi = Trạng thái hiện tại + Ngõ vào w
    // - Moore: z đánh giá chuỗi = Trạng thái hiện tại (không quan tâm ngõ vào)
    if (machine_mode == 1) {
        int len = strlen(temp_str);
        temp_str[len] = input_bit + '0';
        temp_str[len + 1] = '\0';
    }

    int check_len = strlen(temp_str);
    for (int i = 0; i < num_targets; i++) {
        int target_len = strlen(targets[i]);
        if (check_len >= target_len) {
            if (strcmp(temp_str + check_len - target_len, targets[i]) == 0) {
                return 1;
            }
        }
    }
    return 0;
}

void autoBuildStateTable() {
    generateAllStates();
    for (int s = 0; s < num_states; s++) {
        for (int b = 0; b <= 1; b++) {
            state_table[s][b] = calculateNextState(s, b);
            output_table[s][b] = calculateOutput(state_prefixes[s], b);
        }
    }
}

// ---------------------------------------------------------
// CÁC HÀM IN & MÔ PHỎNG
// ---------------------------------------------------------
void printStateTable() {
    printf("\n--- BANG TRANG THAI CHO %d CHUOI (%s) ---\n", num_targets, machine_mode == 0 ? "MOORE" : "MEALY");
    if (machine_mode == 0) {
        printf("HT(State)\tChuoi(Prefix)\tw=0\tw=1\tOutput z\n");
        printf("----------------------------------------------------------\n");
        for (int i = 0; i < num_states; i++) {
            // Đối với Moore, Output ở w=0 và w=1 đều giống nhau
            printf("S%d\t\t%-12s\tS%d\tS%d\t%d\n", 
                   i, strlen(state_prefixes[i]) == 0 ? "(Rong)" : state_prefixes[i],
                   state_table[i][0], state_table[i][1], output_table[i][0]);
        }
    } else {
        printf("HT(State)\tChuoi(Prefix)\tw=0 (S_next, z)\tw=1 (S_next, z)\n");
        printf("----------------------------------------------------------\n");
        for (int i = 0; i < num_states; i++) {
            // Đối với Mealy, Output z đi kèm với quá trình chuyển trạng thái
            printf("S%d\t\t%-12s\tS%d, z=%d\t\tS%d, z=%d\n", 
                   i, strlen(state_prefixes[i]) == 0 ? "(Rong)" : state_prefixes[i],
                   state_table[i][0], output_table[i][0], 
                   state_table[i][1], output_table[i][1]);
        }
    }
}

void simulateAllScenarios() {
    printf("\n=======================================================\n");
    printf(" MO PHONG KICH BAN CHUYEN TRANG THAI\n");
    printf("=======================================================\n\n");
    for (int current_state = 0; current_state < num_states; current_state++) {
        printf(">>> DANG O TRANG THAI [S%d - %s]:\n", 
               current_state, 
               strlen(state_prefixes[current_state]) ? state_prefixes[current_state] : "Rong");

        for (int w = 0; w <= 1; w++) {
            int next_state = state_table[current_state][w];
            int z = output_table[current_state][w];
            printf("\t- w = %d: Chuyen sang ===> [S%d] (Output z = %d)\n", w, next_state, z);
        }
        printf("-------------------------------------------------------\n");
    }
}

int calculateBits(int states) {
    int bits = 0;
    int temp = states - 1; 
    while (temp > 0) { bits++; temp >>= 1; }
    return (bits == 0) ? 1 : bits;
}

void printBinary(int value, int bits) {
    for (int i = bits - 1; i >= 0; i--) printf("%d", (value >> i) & 1);
}

void printStateEncoding() {
    int bits = calculateBits(num_states);
    printf("\n=======================================================\n");
    printf(" MA HOA TRANG THAI (Can dung %d Flip-Flop)\n", bits);
    printf("=======================================================\n");
    for (int i = 0; i < num_states; i++) {
        printf(" Trang thai [S%d] duoc ma hoa thanh bit: ", i);
        printBinary(i, bits);
        printf("\n");
    }
    printf("\n");
}

void printOptimizedEquations() {
    int bits = calculateBits(num_states);
    int total_vars = bits + 1; 
    
    printf("=======================================================\n");
    printf(" PHUONG TRINH LOGIC TOI UU (QUINE-MCCLUSKEY)\n");
    printf("=======================================================\n");

    int dontcares[MAX_MINTERMS];
    int num_d = 0;
    
    int max_s = 1 << bits;
    for (int s = num_states; s < max_s; s++) {
        for (int w = 0; w <= 1; w++) {
            dontcares[num_d++] = (s << 1) | w;
        }
    }

    for (int b = 0; b < bits; b++) {
        int minterms[MAX_MINTERMS];
        int num_m = 0;
        for (int s = 0; s < num_states; s++) {
            for (int w = 0; w <= 1; w++) {
                if (((s >> b) & 1) != ((state_table[s][w] >> b) & 1)) {
                    minterms[num_m++] = (s << 1) | w;
                }
            }
        }
        char func_name[10];
        sprintf(func_name, "T%d", b + 1);
        minimizeQM(func_name, minterms, num_m, dontcares, num_d, total_vars);
    }

    int minterms_z[MAX_MINTERMS];
    int num_mz = 0;
    for (int s = 0; s < num_states; s++) {
        for (int w = 0; w <= 1; w++) {
            if (output_table[s][w] == 1) {
                minterms_z[num_mz++] = (s << 1) | w;
            }
        }
    }
    minimizeQM("z ", minterms_z, num_mz, dontcares, num_d, total_vars);
    printf("=======================================================\n");
}

// ---------------------------------------------------------
// CHƯƠNG TRÌNH CHÍNH
// ---------------------------------------------------------
int main() {
    printf("Chon mo hinh FSM:\n 0 - Moore\n 1 - Mealy\n >> Nhap lua chon (0/1): ");
    scanf("%d", &machine_mode);
    if (machine_mode != 0 && machine_mode != 1) {
        printf("Lua chon khong hop le\n");
        return 1;
    }

    printf("\nNhap so luong chuoi w can phat hien: ");
    scanf("%d", &num_targets);
    
    for (int i = 0; i < num_targets; i++) {
        printf("Nhap chuoi thu %d: ", i + 1);
        scanf("%s", targets[i]);
    }
    // Xây dựng bảng trạng thái tự động dựa trên chuỗi đích và mô hình FSM đã chọn
    autoBuildStateTable();
    printStateTable();

    // Mô phỏng tất cả các kịch bản chuyển trạng thái và ngõ ra z
    simulateAllScenarios();
    printStateEncoding();

    // In ra các minterm cho T-Flip Flop và z, sau đó tối ưu hóa bằng Quine-McCluskey
    printOptimizedEquations();

    return 0;
}