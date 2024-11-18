#pragma once
#include <raylib.h>

typedef struct FileTree {
    const char* path;
    bool is_open;
    bool is_file;
    unsigned int children_count;
    struct FileTree* children;
} FileTree;

FileTree LoadFileTree(const char* path) {
    FileTree tree = { 0 };

    tree.path = malloc(strlen(path) + 1);
    strcpy(tree.path, path);

    //tree.is_open = true;

    FilePathList filePathList = LoadDirectoryFiles(path);

    for (int i = 0; i < filePathList.count; i++) {
        // is directory
        if (!IsPathFile(filePathList.paths[i])) {
            if (tree.children_count == 0) {
                tree.children = (FileTree*)malloc(sizeof(FileTree));
                tree.children_count = 1;
            } else {
                tree.children = (FileTree*)realloc(tree.children, sizeof(FileTree) * (tree.children_count + 1));
                tree.children_count += 1;
            }

            FileTree* new_node = tree.children + tree.children_count - 1;
            *new_node = (FileTree){ 0 };

            const char* new_node_path = (const char*)malloc(strlen(filePathList.paths[i]) + 1);
            strcpy(new_node_path, filePathList.paths[i]);
            new_node->path = new_node_path;
        }
    }

    // TODO: combine these two loop in to one while keeping the files at the bottom of list
    for (int i = 0; i < filePathList.count; i++) {
        // is file
        if (IsPathFile(filePathList.paths[i])) {
            if (tree.children_count == 0) {
                tree.children = (FileTree*)malloc(sizeof(FileTree));
                tree.children_count = 1;
            } else {
                tree.children = (FileTree*)realloc(tree.children, sizeof(FileTree) * (tree.children_count + 1));
                tree.children_count += 1;
            }

            FileTree* new_node = tree.children + tree.children_count - 1;
            *new_node = (FileTree){ 0 };

            const char* new_node_path = (const char*)malloc(strlen(filePathList.paths[i]) + 1);
            strcpy(new_node_path, filePathList.paths[i]);
            new_node->path = new_node_path;
            new_node->is_file = true;
        }
    }

    UnloadDirectoryFiles(filePathList);

    return tree;
}

void CloseFileTree(FileTree* tree) {
    for (int i = 0; i < tree->children_count; i++) {
        free(tree->children[i].path);
        CloseFileTree(&tree->children[i]);
    }

    if (tree->children_count != 0) {
        free(tree->children);
        tree->children_count = 0;
    }
}

void FreeFileTree(FileTree* tree) {
    free(tree->path);

    for (int i = 0; i < tree->children_count; i++) {
        FreeFileTree(&tree->children[i]);
    }
}

void FreeFileTreeInternal(FileTree* tree) {
    free(tree->path);

    for (int i = 0; i < tree->children_count; i++) {
        FreeFileTree(&tree->children[i]);
    }

    // FIXME: IDK if i need this cuz free(tree) already will free all the children cuz recursion
    if (tree->children_count != 0) {
        free(tree->children);
        tree->children_count = 0;
    }

    free(tree);
}

void FileTreeInternal(FileTree* file_tree, int* y_offset, Rectangle bounds, Rectangle old_text_rect, Vector2* scroll, const char** clicked_file_path, int* ret) {
    int spacing = GuiGetStyle(DEFAULT, TEXT_LINE_SPACING);

    Rectangle button_rect;
    button_rect.x = bounds.x;
    button_rect.y = bounds.y + (*y_offset) + scroll->y;
    button_rect.width = bounds.width;
    button_rect.height = spacing;

    Rectangle text_rect;
    text_rect.x = old_text_rect.x + 15;
    text_rect.y = button_rect.y;
    text_rect.width = bounds.width;
    text_rect.height = spacing;

    const char* content = NULL;
    if (!file_tree->is_file) {
        content = GuiIconText(217, GetFileName(file_tree->path));
    } else {
        content = GuiIconText(218, GetFileName(file_tree->path));
    }

    if (GuiButton(button_rect, NULL)) {
        // if directory the expand it
        if (!file_tree->is_file)
        {
            if (file_tree->is_open) {
                CloseFileTree(file_tree);
            } else {
                *file_tree = LoadFileTree(file_tree->path);
            }
        }

        *clicked_file_path = file_tree->path;

        file_tree->is_open = !file_tree->is_open;
        *ret = 1;
    }
    GuiLabel(text_rect, content);

    for (int i = 0; i < file_tree->children_count; i++) {
        *y_offset += spacing;
        FileTreeInternal(&file_tree->children[i], y_offset, (Rectangle) { bounds.x, bounds.y, bounds.width, bounds.height - *y_offset }, text_rect, scroll, clicked_file_path, ret);
    }
}

int GuiFileTree(FileTree* file_tree, Rectangle bounds, Vector2 *scroll, Rectangle *view, const char** clicked_file_path) {
    Rectangle content = { 0 };
    content.width = bounds.width;

    int ret = 0;
    int y_offset = 0;

    int back_color = GuiGetStyle(DEFAULT, BACKGROUND_COLOR);
    DrawRectangleRec(bounds, GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
    GuiSetStyle(DEFAULT, BACKGROUND_COLOR, 0);

    BeginScissorMode(view->x, view->y, view->width, view->height);

    int button_border_width = GuiGetStyle(BUTTON, BORDER_WIDTH);
    GuiSetStyle(BUTTON, BORDER_WIDTH, 0);

    FileTreeInternal(file_tree, &y_offset, (Rectangle){bounds.x, bounds.y, bounds.width, bounds.height}, (Rectangle){ 0 }, scroll, clicked_file_path, &ret);

    EndScissorMode();

    int spacing = GuiGetStyle(DEFAULT, TEXT_LINE_SPACING);
    content.height = y_offset + spacing;

    GuiScrollPanel(bounds, NULL, content, scroll, view);

    GuiSetStyle(BUTTON, BORDER_WIDTH, button_border_width);
    GuiSetStyle(DEFAULT, BACKGROUND_COLOR, back_color);

    return ret;
}