/*
╔════════════════════════════════════════════════════════════╗
║                SFSD Practica Work (TP)                     ║
║              FILE PARTITIONING BY HASHING                  ║
║    By:    Charef Iyad G11 ║ Amer el Khedoud youcef G11     ║
╠════════════════════════════════════════════════════════════╣
*/
#include "functions.h"
#include "stdlib.h"

#ifdef _WIN32
    #define CLEAR_SCREEN "cls"
#else
    #define CLEAR_SCREEN "clear"
#endif

void clear_screen()
{
    system(CLEAR_SCREEN);
}

void display_menu()
{
    clear_screen();
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║      FILE PARTITIONING BY HASHING - MAIN MENU             ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║  1. Create Sample Source File                              ║\n");
    printf("║  2. Partition File by Hashing                              ║\n");
    printf("║  3. Search for a Record                                    ║\n");
    printf("║  4. Insert a Record                                        ║\n");
    printf("║  5. Delete a Record                                        ║\n");
    printf("║  6. Display a Specific Fragment                            ║\n");
    printf("║  7. Display All Fragments                                  ║\n");
    printf("║  8. Display Statistics                                     ║\n");
    printf("║  9. Configure K and M                                      ║\n");
    printf("║  0. Exit                                                   ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("Current Configuration: K = %d fragments, M = %d buffers\n", K_FRAGMENTS, M_BUFFERS);
    printf("\nEnter your choice: ");
}

void clear_input_buffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void create_sample_file_menu()
{
    clear_screen();
    char filename[100];
    int num_records;
    
    printf("\n--- Create Sample Source File ---\n");
    printf("Enter filename (default: source.dat): ");
    fgets(filename, sizeof(filename), stdin);
    filename[strcspn(filename, "\n")] = 0;
    
    if (strlen(filename) == 0) {
        strcpy(filename, "source.dat");
    }
    
    printf("Enter number of records: ");
    scanf("%d", &num_records);
    clear_input_buffer();
    
    create_sample_file(filename, num_records);
}

void partition_file_menu()
{
    clear_screen();
    char filename[100];
    
    printf("\n--- Partition File by Hashing ---\n");
    printf("Enter source filename (default: source.dat): ");
    fgets(filename, sizeof(filename), stdin);
    filename[strcspn(filename, "\n")] = 0;
    
    if (strlen(filename) == 0) {
        strcpy(filename, "source.dat");
    }
    
    partition_file_by_hashing(filename, K_FRAGMENTS, M_BUFFERS);
}

void search_record_menu()
{
    clear_screen();
    char key[MAX_KEY_LEN];
    int fragment_num;
    long block_num;
    int position;
    t_rec record;
    
    printf("\n--- Search for a Record ---\n");
    printf("Enter key to search: ");
    fgets(key, sizeof(key), stdin);
    key[strcspn(key, "\n")] = 0;
    
    if (search_in_fragments(key, K_FRAGMENTS, &fragment_num, &block_num, &position, &record)) {
        printf("\n✓ Record found!\n");
        printf("Fragment: %d\n", fragment_num);
        printf("Block: %ld\n", block_num);
        printf("Position: %d\n", position);
        printf("\nRecord Details:\n");
        printf("  Key: %s\n", record.key);
        printf("  Name: %s\n", record.name);
        printf("  Age: %d\n", record.age);
    } else {
        printf("\n✗ Record with key '%s' not found.\n", key);
    }
}

void insert_record_menu()
{
    clear_screen();
    t_rec record;
    
    printf("\n--- Insert a Record ---\n");
    printf("Enter key: ");
    fgets(record.key, sizeof(record.key), stdin);
    record.key[strcspn(record.key, "\n")] = 0;
    
    printf("Enter name: ");
    fgets(record.name, sizeof(record.name), stdin);
    record.name[strcspn(record.name, "\n")] = 0;
    
    printf("Enter age: ");
    scanf("%d", &record.age);
    clear_input_buffer();
    
    if (insert_into_fragments(record, K_FRAGMENTS)) {
        printf("\n✓ Record inserted successfully!\n");
    } else {
        printf("\n✗ Failed to insert record.\n");
    }
}

void delete_record_menu()
{
    clear_screen();
    char key[MAX_KEY_LEN];
    
    printf("\n--- Delete a Record ---\n");
    printf("Enter key to delete: ");
    fgets(key, sizeof(key), stdin);
    key[strcspn(key, "\n")] = 0;
    
    printf("Are you sure you want to delete record with key '%s'? (y/n): ", key);
    char confirm;
    scanf("%c", &confirm);
    clear_input_buffer();
    
    if (confirm == 'y' || confirm == 'Y') {
        if (delete_from_fragments(key, K_FRAGMENTS)) {
            printf("\n✓ Record deleted successfully!\n");
        } else {
            printf("\n✗ Failed to delete record.\n");
        }
    } else {
        printf("Deletion cancelled.\n");
    }
}

void display_fragment_menu()
{
    clear_screen();
    int fragment_num;
    
    printf("\n--- Display a Specific Fragment ---\n");
    printf("Enter fragment number (0 to %d): ", K_FRAGMENTS - 1);
    scanf("%d", &fragment_num);
    clear_input_buffer();
    
    if (fragment_num >= 0 && fragment_num < K_FRAGMENTS) {
        clear_screen();
        display_fragment(fragment_num);
    } else {
        printf("Invalid fragment number!\n");
    }
}

void configure_k_m_menu()
{
    clear_screen();
    int k, m;
    
    printf("\n--- Configure K and M ---\n");
    printf("Current values: K = %d, M = %d\n", K_FRAGMENTS, M_BUFFERS);
    
    printf("Enter new value for K (number of fragments): ");
    scanf("%d", &k);
    
    printf("Enter new value for M (number of buffers, must satisfy 2 < M < K): ");
    scanf("%d", &m);
    clear_input_buffer();
    
    if (m > 2 && m < k) {
        K_FRAGMENTS = k;
        M_BUFFERS = m;
        printf("\n✓ Configuration updated: K = %d, M = %d\n", K_FRAGMENTS, M_BUFFERS);
    } else {
        printf("\n✗ Invalid configuration! Must satisfy: 2 < M < K\n");
    }
}

int main()
{
    clear_screen();
    int choice;
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║         FILE PARTITIONING BY HASHING SYSTEM                ║\n");
    printf("║         SFSD - 2CP - ESI - 2024/2025                       ║\n");
    printf("║    By:    Charef Iyad G11 ║ Amer el Khedoud youcef G11     ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    while (1) {
        display_menu();
        scanf("%d", &choice);
        clear_input_buffer();
        
        switch (choice) {
            case 1:
                create_sample_file_menu();
                break;
            case 2:
                partition_file_menu();
                break;
            case 3:
                search_record_menu();
                break;
            case 4:
                insert_record_menu();
                break;
            case 5:
                delete_record_menu();
                break;
            case 6:
                display_fragment_menu();
                break;
            case 7:
                clear_screen();
                display_all_fragments(K_FRAGMENTS);
                break;
            case 8:
                clear_screen();
                display_statistics(K_FRAGMENTS);
                break;
            case 9:
                configure_k_m_menu();
                break;
            case 0:
                clear_screen();
                printf("\nExiting program. Goodbye!\n");
                return 0;
            default:
                printf("\n✗ Invalid choice! Please try again.\n");
        }
        
        printf("\nPress Enter to continue...");
        getchar();
        clear_screen();
    }
    
    return 0;

}
