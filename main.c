#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define WIDTH 80
#define HEIGHT 24
#define MAX_OBJECTS 200

typedef enum { NONE=0, CIRCLE, RECTANGLE, LINE, TRIANGLE } ObjType;

typedef struct {
    int id;
    ObjType type;
    // params: we'll use generic fields depending on type
    int x1,y1,x2,y2,x3,y3; // coordinates
    int r; // radius for circle
    int in_use;
} Object;

char canvas[HEIGHT][WIDTH];
Object objects[MAX_OBJECTS];
int next_id = 1;

void clear_canvas() {
    for (int y=0;y<HEIGHT;y++)
        for (int x=0;x<WIDTH;x++)
            canvas[y][x] = '_';
}

void print_canvas() {
    for (int y=0;y<HEIGHT;y++) {
        for (int x=0;x<WIDTH;x++) putchar(canvas[y][x]);
        putchar('\n');
    }
}

void set_pixel(int x,int y) {
    if (x>=0 && x<WIDTH && y>=0 && y<HEIGHT) canvas[y][x] = '*';
}

// Bresenham line
void draw_line_coords(int x0,int y0,int x1,int y1) {
    int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
    int dy = -abs(y1-y0), sy = y0<y1 ? 1 : -1;
    int err = dx+dy, e2;
    while (1) {
        set_pixel(x0,y0);
        if (x0==x1 && y0==y1) break;
        e2 = 2*err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

// Midpoint circle algorithm
void draw_circle_coords(int xc,int yc,int r) {
    if (r<0) return;
    int x = 0; int y = r; int d = 1 - r;
    while (x <= y) {
        set_pixel(xc + x, yc + y);
        set_pixel(xc - x, yc + y);
        set_pixel(xc + x, yc - y);
        set_pixel(xc - x, yc - y);
        set_pixel(xc + y, yc + x);
        set_pixel(xc - y, yc + x);
        set_pixel(xc + y, yc - x);
        set_pixel(xc - y, yc - x);
        if (d < 0) d += 2*x + 3;
        else { d += 2*(x - y) + 5; y--; }
        x++;
    }
}

void draw_rectangle_coords(int x1,int y1,int x2,int y2) {
    // draw border of axis-aligned rectangle
    int left = x1<x2?x1:x2, right = x1<x2?x2:x1;
    int top = y1<y2?y1:y2, bottom = y1<y2?y2:y1;
    for (int x=left;x<=right;x++) { set_pixel(x, top); set_pixel(x, bottom); }
    for (int y=top;y<=bottom;y++) { set_pixel(left, y); set_pixel(right, y); }
}

void draw_triangle_coords(int x1,int y1,int x2,int y2,int x3,int y3) {
    draw_line_coords(x1,y1,x2,y2);
    draw_line_coords(x2,y2,x3,y3);
    draw_line_coords(x3,y3,x1,y1);
}

void redraw_all() {
    clear_canvas();
    for (int i=0;i<MAX_OBJECTS;i++) {
        if (!objects[i].in_use) continue;
        Object *o = &objects[i];
        switch (o->type) {
            case LINE:
                draw_line_coords(o->x1,o->y1,o->x2,o->y2); break;
            case RECTANGLE:
                draw_rectangle_coords(o->x1,o->y1,o->x2,o->y2); break;
            case CIRCLE:
                draw_circle_coords(o->x1,o->y1,o->r); break;
            case TRIANGLE:
                draw_triangle_coords(o->x1,o->y1,o->x2,o->y2,o->x3,o->y3); break;
            default: break;
        }
    }
}

int add_object(Object *o) {
    for (int i=0;i<MAX_OBJECTS;i++) {
        if (!objects[i].in_use) {
            objects[i] = *o;
            objects[i].in_use = 1;
            objects[i].id = next_id++;
            return objects[i].id;
        }
    }
    return -1; // full
}

Object* find_object_by_id(int id) {
    for (int i=0;i<MAX_OBJECTS;i++) if (objects[i].in_use && objects[i].id==id) return &objects[i];
    return NULL;
}

int delete_object(int id) {
    for (int i=0;i<MAX_OBJECTS;i++) if (objects[i].in_use && objects[i].id==id) { objects[i].in_use=0; return 1; }
    return 0;
}

void list_objects() {
    printf("Objects:\n");
    for (int i=0;i<MAX_OBJECTS;i++) if (objects[i].in_use) {
        Object *o = &objects[i];
        switch (o->type) {
            case LINE: printf("ID %d: LINE (%d,%d)-(%d,%d)\n", o->id,o->x1,o->y1,o->x2,o->y2); break;
            case RECTANGLE: printf("ID %d: RECT (%d,%d)-(%d,%d)\n", o->id,o->x1,o->y1,o->x2,o->y2); break;
            case CIRCLE: printf("ID %d: CIRCLE center=(%d,%d) r=%d\n", o->id,o->x1,o->y1,o->r); break;
            case TRIANGLE: printf("ID %d: TRI (%d,%d)-(%d,%d)-(%d,%d)\n", o->id,o->x1,o->y1,o->x2,o->y2,o->x3,o->y3); break;
            default: break;
        }
    }
}

void prompt_add() {
    printf("Choose object to add: 1)Circle 2)Rectangle 3)Line 4)Triangle\n");
    int c; if (scanf("%d",&c)!=1) { while(getchar()!='\n'); return; }
    Object o; memset(&o,0,sizeof(o));
    if (c==1) {
        o.type = CIRCLE;
        printf("Enter center x y and radius: "); scanf("%d %d %d", &o.x1,&o.y1,&o.r);
    } else if (c==2) {
        o.type = RECTANGLE;
        printf("Enter corner1 x y and corner2 x y: "); scanf("%d %d %d %d", &o.x1,&o.y1,&o.x2,&o.y2);
    } else if (c==3) {
        o.type = LINE;
        printf("Enter x1 y1 x2 y2: "); scanf("%d %d %d %d", &o.x1,&o.y1,&o.x2,&o.y2);
    } else if (c==4) {
        o.type = TRIANGLE;
        printf("Enter x1 y1 x2 y2 x3 y3: "); scanf("%d %d %d %d %d %d", &o.x1,&o.y1,&o.x2,&o.y2,&o.x3,&o.y3);
    } else { printf("Invalid choice\n"); return; }
    int id = add_object(&o);
    if (id<0) printf("Object list full\n"); else { printf("Added with ID %d\n", id); redraw_all(); }
}

void prompt_delete() {
    printf("Enter object ID to delete: "); int id; if (scanf("%d",&id)!=1) { while(getchar()!='\n'); return; }
    if (delete_object(id)) { printf("Deleted %d\n",id); redraw_all(); }
    else printf("No object with ID %d\n", id);
}

void prompt_modify() {
    printf("Enter object ID to modify: "); int id; if (scanf("%d",&id)!=1) { while(getchar()!='\n'); return; }
    Object *o = find_object_by_id(id);
    if (!o) { printf("Not found\n"); return; }
    switch (o->type) {
        case CIRCLE:
            printf("Enter new center x y and radius: "); scanf("%d %d %d", &o->x1,&o->y1,&o->r); break;
        case RECTANGLE:
            printf("Enter new corner1 x y and corner2 x y: "); scanf("%d %d %d %d", &o->x1,&o->y1,&o->x2,&o->y2); break;
        case LINE:
            printf("Enter new x1 y1 x2 y2: "); scanf("%d %d %d %d", &o->x1,&o->y1,&o->x2,&o->y2); break;
        case TRIANGLE:
            printf("Enter new x1 y1 x2 y2 x3 y3: "); scanf("%d %d %d %d %d %d", &o->x1,&o->y1,&o->x2,&o->y2,&o->x3,&o->y3); break;
        default: break;
    }
    printf("Modified %d\n", id);
    redraw_all();
}

void print_menu() {
    printf("\n2D ASCII Graphics Editor\n");
    printf("Canvas: %dx%d (origin 0,0 top-left)\n", WIDTH, HEIGHT);
    printf("1) Display picture\n");
    printf("2) List objects\n");
    printf("3) Add object\n");
    printf("4) Delete object\n");
    printf("5) Modify object\n");
    printf("6) Clear all objects\n");
    printf("7) Exit\n");
    printf("Choose: ");
}

int main(void) {
    memset(objects,0,sizeof(objects));
    clear_canvas();
    int choice;
    while (1) {
        print_menu();
        if (scanf("%d", &choice)!=1) { while(getchar()!='\n'); continue; }
        if (choice==1) {
            print_canvas();
        } else if (choice==2) {
            list_objects();
        } else if (choice==3) {
            prompt_add();
        } else if (choice==4) {
            prompt_delete();
        } else if (choice==5) {
            prompt_modify();
        } else if (choice==6) {
            for (int i=0;i<MAX_OBJECTS;i++) objects[i].in_use=0; next_id=1; clear_canvas(); printf("Cleared all objects\n");
        } else if (choice==7) {
            break;
        } else {
            printf("Invalid choice\n");
        }
    }
    return 0;
}
