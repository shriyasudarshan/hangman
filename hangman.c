#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_WORD_LENGTH 64
#define ALPHABET_SIZE 26

/* Maximum wrong guesses allowed per round */
#define MAX_TRIES 6

/* Struct to hold a word and its hint */
typedef struct {
    char word[MAX_WORD_LENGTH];
    char hint[MAX_WORD_LENGTH];
} WordWithHint;

/* Prototypes */
void drawHangman(int wrong);
void displayWordProgress(const char secret[], const bool revealed[]);
void toLowerStr(char s[]);
int chooseDifficultyAndIndex(int *wordCount);
bool playRound(const WordWithHint words[], int wordCount, int difficulty, int *score);

int main(void) {
    srand((unsigned)time(NULL));

    /* Word lists grouped roughly by difficulty.
       In real usage you can expand these lists or load from a file. */
    WordWithHint easyWords[] = {
        {"pizza", "A popular Italian dish"},
        {"beach", "Sandy shore by the sea"},
        {"apple", "A common fruit"},
        {"cat", "A small pet that purrs"},
        {"book", "You read this"}
    };
    WordWithHint mediumWords[] = {
        {"elephant", "A large mammal with a trunk"},
        {"notebook", "Used to write notes"},
        {"computer", "Electronic machine for computation"},
        {"hangman", "This game's name"},
        {"galaxy", "A system of millions or billions of stars"}
    };
    WordWithHint hardWords[] = {
        {"geeksforgeeks", "Computer coding"},
        {"cryptography", "Study of secure communication"},
        {"quizzical", "Showing puzzlement"},
        {"photosynthesis", "How plants make food"},
        {"metacognition", "Thinking about thinking"}
    };

    /* Aggregate pointers for convenience */
    WordWithHint *allWords[3];
    int counts[3];
    allWords[0] = easyWords; counts[0] = sizeof(easyWords)/sizeof(easyWords[0]);
    allWords[1] = mediumWords; counts[1] = sizeof(mediumWords)/sizeof(mediumWords[0]);
    allWords[2] = hardWords; counts[2] = sizeof(hardWords)/sizeof(hardWords[0]);

    printf("Welcome to the improved Hangman game!\n\n");

    int score = 0;
    bool keepPlaying = true;

    while (keepPlaying) {
        printf("Choose difficulty (1 = Easy, 2 = Medium, 3 = Hard): ");
        int difficulty = 0;
        if (scanf("%d", &difficulty) != 1) {
            /* clear input */
            while (getchar() != '\n');
            printf("Invalid input. Defaulting to Medium.\n");
            difficulty = 2;
        }
        if (difficulty < 1 || difficulty > 3) difficulty = 2;

        /* play one round */
        bool won = playRound(allWords[difficulty-1], counts[difficulty-1], difficulty, &score);

        if (won) {
            printf("Round result: YOU WON! Current score: %d\n", score);
        } else {
            printf("Round result: YOU LOST. Current score: %d\n", score);
        }

        printf("\nDo you want to play again? (y/n): ");
        char choice = '\0';
        while ( (choice = (char)tolower(getchar())) != EOF) {
            if (choice == '\n') continue;
            /* consume rest of line */
            while (getchar() != '\n');
            break;
        }
        if (choice != 'y') keepPlaying = false;
        printf("\n------------------------------------------\n\n");
    }

    printf("Thanks for playing! Final score: %d\n", score);
    return 0;
}

/* playRound: plays one round with a randomly selected word from the given list.
   Returns true if player guessed the word, false otherwise.
   Updates *score: +10 for win, -5 for loss (simple example). */
bool playRound(const WordWithHint words[], int wordCount, int difficulty, int *score) {
    if (wordCount <= 0) return false;

    /* pick random word */
    int idx = rand() % wordCount;
    const char *secretRaw = words[idx].word;
    const char *hint = words[idx].hint;

    /* create lowercase copy of secret */
    char secret[MAX_WORD_LENGTH];
    strncpy(secret, secretRaw, sizeof(secret)-1);
    secret[sizeof(secret)-1] = '\0';
    toLowerStr(secret);

    int len = (int)strlen(secret);

    /* revealed[] tracks which characters are revealed (true for revealed) */
    bool revealed[MAX_WORD_LENGTH] = { false };

    /* initialize revealed for non-alpha chars (like hyphens) */
    for (int i = 0; i < len; ++i) {
        if (!isalpha((unsigned char)secret[i])) revealed[i] = true;
    }

    /* guessedLetters tracks letters already guessed */
    bool guessedLetters[ALPHABET_SIZE] = { false };

    int wrong = 0;
    int remainingToGuess = 0;
    for (int i = 0; i < len; ++i) if (isalpha((unsigned char)secret[i])) remainingToGuess++;

    printf("\nHint: %s\n", hint);
    printf("Difficulty: %s | Allowed wrong guesses: %d\n",
           (difficulty == 1 ? "Easy" : (difficulty == 2 ? "Medium" : "Hard")),
           MAX_TRIES);

    /* Initialize guessed word display (not needed separately since we use revealed[]) */

    /* Game loop */
    while (wrong < MAX_TRIES && remainingToGuess > 0) {
        printf("\n");
        drawHangman(wrong);
        displayWordProgress(secret, revealed);

        /* show guessed letters */
        printf("Guessed letters: ");
        bool anyGuessed = false;
        for (int i = 0; i < ALPHABET_SIZE; ++i) {
            if (guessedLetters[i]) {
                printf("%c ", 'a' + i);
                anyGuessed = true;
            }
        }
        if (!anyGuessed) printf("(none)");
        printf("\n");

        printf("Wrong guesses left: %d\n", MAX_TRIES - wrong);
        printf("Enter a letter: ");

        /* Read a single non-whitespace char from stdin, validate */
        char buffer[64];
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            /* input error; treat as quit */
            printf("\nInput error. Exiting round.\n");
            return false;
        }

        /* find first non-space char */
        char raw = '\0';
        for (size_t i = 0; i < strlen(buffer); ++i) {
            if (!isspace((unsigned char)buffer[i])) {
                raw = buffer[i];
                break;
            }
        }

        if (raw == '\0') {
            printf("No input detected. Please type a single letter.\n");
            continue;
        }

        if (!isalpha((unsigned char)raw)) {
            printf("Please enter a letter (a-z).\n");
            continue;
        }

        char guess = (char)tolower((unsigned char)raw);

        int idxLetter = guess - 'a';
        if (idxLetter < 0 || idxLetter >= ALPHABET_SIZE) {
            printf("Letter out of range. Try again.\n");
            continue;
        }

        if (guessedLetters[idxLetter]) {
            printf("You've already guessed '%c'. Try a different letter.\n", guess);
            continue;
        }

        guessedLetters[idxLetter] = true;

        /* Check if the guessed letter is in the secret */
        bool found = false;
        for (int i = 0; i < len; ++i) {
            if (secret[i] == guess && !revealed[i]) {
                revealed[i] = true;
                remainingToGuess--;
                found = true;
            }
        }

        if (found) {
            printf("Good guess! '%c' is in the word.\n", guess);
        } else {
            printf("Sorry, '%c' is NOT in the word.\n", guess);
            wrong++;
        }
    }

    /* final state */
    if (remainingToGuess == 0) {
        drawHangman(wrong);
        displayWordProgress(secret, revealed);
        printf("\nCongratulations — you guessed the word: %s\n", secret);
        *score += 10;
        return true;
    } else {
        /* player used up attempts */
        drawHangman(MAX_TRIES);
        /* reveal full word */
        printf("\nOut of guesses. The word was: %s\n", secret);
        *score -= 5;
        return false;
    }
}

/* drawHangman: prints ASCII hangman depending on how many wrong guesses have occurred
   Accepts wrong in [0 .. MAX_TRIES]. The array contains MAX_TRIES+1 lines/stages. */
void drawHangman(int wrong) {
    const char *stages[] = {
        "     _________      ",
        "    |         |     ",
        "    |         O     ",
        "    |        /|\\   ",
        "    |        / \\   ",
        "    |               ",
        "   _|_              "
    };
    int stagesCount = (int)(sizeof(stages) / sizeof(stages[0]));

    /* We'll print up to the stage corresponding to wrong, but bound safely */
    int toPrint = wrong;
    if (toPrint < 0) toPrint = 0;
    if (toPrint >= stagesCount) toPrint = stagesCount - 1;

    printf("\n");
    for (int i = 0; i <= toPrint; ++i) {
        printf("%s\n", stages[i]);
    }
}

/* displayWordProgress: prints the secret word with underscores for unrevealed letters */
void displayWordProgress(const char secret[], const bool revealed[]) {
    printf("Word: ");
    for (int i = 0; secret[i] != '\0'; ++i) {
        if (revealed[i]) {
            printf("%c ", secret[i]);
        } else if (isalpha((unsigned char)secret[i])) {
            printf("_ ");
        } else {
            /* Non-alpha characters printed directly (space, dash, etc.) */
            printf("%c ", secret[i]);
        }
    }
    printf("\n");
}

/* toLowerStr: converts string in-place to lowercase */
void toLowerStr(char s[]) {
    for (size_t i = 0; i < strlen(s); ++i) s[i] = (char)tolower((unsigned char)s[i]);
}
