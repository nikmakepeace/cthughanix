#include "cthugha.h"
#include "translate.h"
#include "display.h"
#include "Interface.h"
#include "tab_header.h"
#include "cth_buffer.h"
#include "CthughaBuffer.h"

#include <stdint.h>
#include <vector>

OptionOnOff use_translates("use-translate", 1); /* allow translations */
OptionOnOff trans_stretch("stretching", 1); /* allow stretching */

static CoreOptionEntry* _trans[] = { new TranslateEntry("none", "No Translate") };
static CoreOptionEntryList translateEntries(_trans, 1);
TranslateOption translation(-1, "translate");

char lib_size[512];
static const char* translate_path[] = { "./", "./resources/tab/", lib_size, CTH_LIBDIR "/tab/", "" };

/*
 * Initialize the translate-tables.
 */
int init_translate(const CthughaBuffer& buffer) {

    if (use_translates) {

        CTH_INFO("  loading translation tables...\n");

        sprintf(lib_size, CTH_LIBDIR "/tab/%dx%d/", buffer.width(), buffer.height());

        TranslateLoadTarget target(buffer.width(), buffer.height());
        translation.load(translate_path, "/tab/", ".cmd", TranslateEntry::loaderCmd, &target);
        translation.load(translate_path, "/tab/", ".tab", TranslateEntry::loaderTab, &target);

        CTH_INFO("  number of loaded translates: %d\n",
            translation.getNEntries());
    }

    return 0;
}

void TranslateEntry::setTable(const int* table, int count, int width, int height) {
    if (table == 0 || count <= 0) {
        tableData.clear();
        widthValue = 0;
        heightValue = 0;
        return;
    }

    tableData.assign(table, table + count);
    widthValue = width;
    heightValue = height;
}

void TranslateEntry::setTable(std::vector<int>& table, int width, int height) {
    tableData.swap(table);
    widthValue = width;
    heightValue = height;
}

//
// Read a translate file
//
static int* read_trans_data(FILE* file, const tab_header& header, int BSize, const char* name) {
    union data {
        uint32_t* l;
        uint16_t* s;
    } D;
    int i, j;

    int size = (BSize > 65535) ? sizeof(uint32_t) : sizeof(uint16_t);

    int* trans = new int[BSize];
    int* dst = trans;

    /* allocate memory for read buffer */
    D.l = new uint32_t[header.size_x];
    if ((void*)D.l != (void*)D.s) {
        CTH_ERROR("Wackiness afoot at %d in %s\n", __LINE__, __FILE__);
        exit(1);
    }

    /* read data */
    for (i = 0; i < header.size_y; i++) {
        if ((j = fread(D.l, size, header.size_x, file)) < header.size_x) {
            CTH_ERROR("  Can't read at line: %d, read: %d (%s)\n", i, j, name);
            delete[] D.l;
            delete[] trans;
            return NULL;
        }
        for (j = 0; j < header.size_x; j++) {
            if (BSize > 65535) {
                if (D.l[j] >= (unsigned int)(BSize)) {
                    CTH_ERROR("  High-translate (value: %u) in %s.\n", D.l[j], name);
                    delete[] D.l;
                    delete[] trans;
                    return NULL;
                }
                *dst++ = (int)D.l[j];
            } else {
                if (D.s[j] >= (unsigned int)(BSize)) {
                    CTH_ERROR("  High-translate (value: %d) in %s.\n", D.s[j], name);
                    delete[] D.l;
                    delete[] trans;
                    return NULL;
                }
                *dst++ = (int)D.s[j];
            }
        }
    }

    delete[] D.l;

    /* Check for too much data */
    if (fread(&D, size, 1, file)) {
        CTH_ERROR("  Extra data at end of file %s\n", name);
        delete[] trans;
        return NULL;
    }

    return trans;
}

static int* stretch_trans(const int* src, const tab_header& header,
    const TranslateLoadTarget& target) {
    double xs, ys;
    int x, y, tp, ox, oy, dx, dy;
    int i, j;

    CTH_DEBUG(" ... stretching");

    int* dst = new int[target.size];

    xs = (double)header.size_x / target.width;
    ys = (double)header.size_y / target.height;

    for (j = 0; j < target.height; j++) {

        y = int((double)j * ys);
        if (y >= header.size_y)
            y = header.size_y - 1;

        for (i = 0; i < target.width; i++) {

            x = int((double)i * xs);
            if (x >= header.size_x)
                x = header.size_x - 1;

            tp = src[x + y * header.size_x];
            ox = tp % header.size_x;
            oy = tp / header.size_x;
            dx = int((double)(ox - x) / xs);
            dy = int((double)(oy - y) / ys);
            dst[i + j * target.width] = abs(i + dx + (j + dy) * target.width) % target.size;
        }
    }
    return dst;
}

CoreOptionEntry* TranslateEntry::loaderTab(
    FILE* file, const char* name, const char* dir, const char* total_name, void* context) {
    (void)dir;
    (void)total_name;

    tab_header header;

    const TranslateLoadTarget& target = *static_cast<const TranslateLoadTarget*>(context);
    int BSize = target.size;
    int stretch = 0;
    TranslateEntry* new_trans;

    /* read header */
    if (fread(&header, sizeof(tab_header), 1, file) != 1) {
        CTH_ERROR("  Can't read header from file '%s'.\n", name);
        return NULL;
    }

    /* check header ID */
    if (!tab_header_has_id(&header)) {
        CTH_WARN("\n  Header ID-mismatch. Trying without header.");

        rewind(file); // back to start of file

        /* fill in header */
        header.size_x = target.width;
        header.size_y = target.height;

        new_trans = new TranslateEntry(name, "");
    } else {
        /* ID OK - now test size */
        if ((header.size_x != target.width) || (header.size_y != target.height)) {
            CTH_WARN("\n    Size mismatch (%dx%d instead of %dx%d)", header.size_x, header.size_y,
                target.width, target.height);
            if (int(trans_stretch)) { /* allow stretching */
                stretch = 1;
            } else {
                return NULL;
            }
        }

        /* translate table might use a different size */
        BSize = header.size_x * header.size_y;

        // make sure there is no \n in the name
        char* ent = strchr(header.description, '\n');
        if (ent != NULL)
            *ent = '\0';

        new_trans = new TranslateEntry(name, header.description);
    }

    /* read data */
    int* trans = read_trans_data(file, header, BSize, name);
    if (trans == NULL) {
        delete new_trans;
        return NULL;
    }

    /* do the stretching if necessary (from: Rus Maxham) */
    if (stretch) {
        int* stretched = stretch_trans(trans, header, target);
        new_trans->setTable(stretched, target.size, target.width, target.height);
        delete[] stretched;
        delete[] trans;
    } else {
        new_trans->setTable(trans, BSize, target.width, target.height);
        delete[] trans;
    }

    return new_trans;
}

/*
 * read a file in the form:
 *  cmdtab
 *  <descrption>
 *  <commd> [args] %d %d
 */
#define MAX_DESC_LEN 64

static int read_generated_trans_data(
    FILE* in, const TranslateLoadTarget& target, std::vector<int>& trans, const char* name) {
    trans.assign(target.size, 0);
    std::vector<uint32_t> line(target.width);

    for (int y = 0; y < target.height; y++) {
        if (fread(&line[0], sizeof(uint32_t), target.width, in) != (size_t)target.width) {
            CTH_ERRNO(errno, "  reading translation table %s. failed at line %d.", name, y);
            return 1;
        }

        int* dst = &trans[target.width * y];
        for (int x = 0; x < target.width; x++, dst++) {
            if (line[x] >= (uint32_t)target.size) {
                CTH_ERROR("  illegal value in translation table %s.\n", name);
                return 1;
            }
            *dst = (int)line[x];
        }
    }

    return 0;
}

CoreOptionEntry* TranslateEntry::loaderCmd(
    FILE* file, const char* name, const char* dir, const char* total_name, void* context) {
    (void)total_name;

    char line[PATH_MAX];
    char command[PATH_MAX];
    FILE* cmd_file;
    TranslateEntry* new_trans;
    const TranslateLoadTarget& target = *static_cast<const TranslateLoadTarget*>(context);

    /* check ID */
    fgets(line, 512, file);
    if (strncmp(line, "cmdtab", 6) != 0) {
        CTH_ERROR("  Not a command translate file: %s.\n", name);
        return NULL;
    }

    // get description
    fgets(line, MAX_DESC_LEN, file);
    line[MAX_DESC_LEN - 1] = '\0'; // make sure the string terminates

    if (strchr(line, '\n') != NULL) // delete trailing \n
        *(strchr(line, '\n')) = '\0';

    new_trans = new TranslateEntry(name, line);

    /* get command */
    fgets(line, PATH_MAX, file);

    /* generate the command line */
    char cmd2[PATH_MAX];
    if (snprintf(cmd2, sizeof(cmd2), "%s/%s", dir, line) >= (int)sizeof(cmd2)) {
        CTH_ERROR("  Translation command path too long in %s.\n", name);
        return NULL;
    }
    if (snprintf(command, sizeof(command), cmd2, target.width, target.height) >= (int)sizeof(command)) {
        CTH_ERROR("  Translation command too long in %s.\n", name);
        return NULL;
    }

    if (strchr(command, '\n') != NULL) { // delete trailing '\n`
        *(strchr(command, '\n')) = '\0';
    }

    /* start the command */
    CTH_DEBUG("\n    starting: %s", command);
    if ((cmd_file = popen(command, "r")) == NULL) {
        CTH_ERRNO(errno, "  Can't run command '%s'.\n", command);
        delete new_trans;
        return NULL;
    }

    std::vector<int> generated;

    if (read_generated_trans_data(cmd_file, target, generated, name)) {
        pclose(cmd_file);
        delete new_trans;
        return NULL;
    }

    new_trans->setTable(generated, target.width, target.height);

    pclose(cmd_file);

    return new_trans;
}

TranslateOption::TranslateOption(int buffer, const char* name)
    : CoreOption(buffer, name, translateEntries) { }

TranslateEntry* TranslateOption::translateEntry(int index) {
    if ((index < 0) || (index >= getNEntries()))
        return NULL;

    return static_cast<TranslateEntry*>(entries[index]);
}

TranslateEntry* TranslateOption::currentTranslateEntry() {
    return translateEntry(currentN());
}

TranslationTable TranslateOption::translationTable(int index) {
    TranslateEntry* entry = translateEntry(index);
    return (entry != 0) ? entry->table() : TranslationTable();
}

TranslationTable TranslateOption::currentTranslationTable() {
    return translationTable(currentN());
}
