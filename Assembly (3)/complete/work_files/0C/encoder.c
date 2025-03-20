
#define STDIN 0
#define STDOUT 1
#define STDERR 2
#define SYS_READ 3
#define SYS_WRITE 4
#define SYS_OPEN 5
#define SYS_CLOSE 6

int debug_mode = 1;
int is_add = 1;
int infile = STDIN; //changed FILE*
int outfile = STDOUT; //changed FILE*
char* encoding_key = "0";
int key_position = 0;
int error_flag = 0;

// ====== TAKEN FROM MY LAB A - modified some functions to work low level======

// character value if it is in the range 'A' to 'z' (no encoding otherwise)
int should_encode(char c){
    return ((c >= 'A' && c <= 'z'));
}

// /*
//  *   The funciton will *shift* the current *char*. If it's a digit will wrap around 0 to 9, and if it's a letter a to z. 
//  *   meaning: 9 + 1 = 0, A - 1 = Z...
//  *   key_position = keeps incrementing (happends only after should_encode) until we are out of encoding keys and then wraps around to position 0 and starts over
// */
// char wrap_around(char c, int shift){
//     if (c >= '0' && c <= '9') {
//         c = '0' + (c - '0' + shift + 10)%10;
//     }
//     else if(c >= 'A' && c <= 'Z'){ 
//         c = 'A' + (c - 'A' + shift + 26)%26;
//     }
//     else{
//         c = 'a' + (c - 'a' + shift + 26)%26;
//     }
//     return c;
// }

/*
 * The encoder will be a simplified version that reads a character (from stdin by default), 
 encodes it by adding 1 to the character value if it is in the range 'A' to 'z' 
 (no encoding otherwise), and outputs it (to stdout by default). 
*/
char encode (char c){
    // int shift = 0;
    // shift = encoding_key[key_position] - '0'; // from char to integer
    // key_position = encoding_key[key_position + 1] ? key_position + 1 : 0;
    // shift = (is_add) ? shift : -shift;
    c = (should_encode(c)) ? (c+1) : c;
    return c;
}



void parse_args(int argc, char **argv){
    // infile = stdin;outfile = stdout; ---> removed to work low level
    int i; //can't declare inside for now.

    for(i = 1; i < argc; i++){ //not printing ./encoder (just i = 0 to print it...)
        // if(argv[i][0] == '-' && argv[i][1]== 'D') debug_mode = 0;
        // else if(argv[i][0] == '+' && argv[i][1]== 'D') debug_mode = 1;
        //else
        if(argv[i][0] == '+' && argv[i][1] =='E'){
            is_add = 1;
            encoding_key = &argv[i][2];
            key_position = 0;
        }
        else if(argv[i][0] == '-' && argv[i][1] =='E'){
            is_add = 0;
            encoding_key = &argv[i][2];
            key_position = 0;
        }
        else if(argv[i][0] == '-' && argv[i][1] == 'i'){
            infile = system_call(SYS_OPEN, argv[i]+2, 0, 0644); // <--- like fopen. === OPEN (argv[i]+2 = filename), permissions:readonly
            if(infile < 0) {
                system_call(SYS_WRITE, STDERR, "Cannot open input file\n", 21);
                system_call(1, 0x55);  
            }
        }
        else if(argv[i][0] == '-' && argv[i][1] == 'o'){
            outfile = system_call(SYS_OPEN, argv[i]+2, 1, 0644); // 01101 = write only + create if not existed + reset file in beginning.
            if(outfile < 0) {
                system_call(SYS_WRITE, STDERR, "Cannot open output file\n", 22);
                system_call(1, 0x55);
            }
        }
        else if(debug_mode){                        // DEBUG MODES: if you want to see the above in debug -> remove the else in the beginning of this line. :)
            //fprintf(stderr,"%s\n", argv[i]);
            system_call(SYS_WRITE, STDERR, argv[i], strlen(argv[i]));
            system_call(SYS_WRITE, STDERR, "\n", 1);
        }
    }
}

// Process input character by character ---> changed to low level
void process_inputs(){
    // int c;
    // while((c = fgetc(infile)) != EOF){ //ctrl+D. = EOF
    //     fputc(encode(c),outfile);
    // }
    // printf("\n");
    char buffer[1];
    int count;

    while((count = system_call(SYS_READ,infile,buffer,1)) > 0){
        buffer[0] = encode(buffer[0]);;
        system_call(SYS_WRITE, outfile, buffer, 1);
    }
}

// Close output streams "normally"
void close_output_stream(){
    if(outfile != STDOUT){
        system_call(SYS_CLOSE, outfile);;
    }
}


int main(int argc, char **argv) {

    parse_args(argc,argv);
    if(error_flag){return 1;}
    process_inputs();
    close_output_stream();
    
    return 0;
}