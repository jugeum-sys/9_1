#define _CRT_SECURE_NO_WARNINGS   
#include <stdio.h>

#define V             10      
#define E             20      
#define MAX_WEIGHT   100      
#define INF   1000000000      
#define DEFAULT_SEED 12345


static unsigned long rngState = DEFAULT_SEED;
static void rng_seed(unsigned long seed) { rngState = seed; }
static int rng_next(int bound) {
    rngState = rngState * 1103515245UL + 12345UL;
    return (int)((rngState >> 16) % (unsigned long)bound);
}


typedef struct { int u, v, w; } Edge;

static Edge edges[E];                 
static int  weight[V][V];             
static int  edgeCount = 0;

static void add_edge(int u, int v, int w) {
    weight[u][v] = w;
    weight[v][u] = w;                 
    edges[edgeCount].u = u;
    edges[edgeCount].v = v;
    edges[edgeCount].w = w;
    edgeCount++;
}

static void generate_graph(void) {
    int i, u, v, w;

    for (u = 0; u < V; u++)
        for (v = 0; v < V; v++) weight[u][v] = 0;
    edgeCount = 0;

    for (i = 1; i < V; i++) {
        u = rng_next(i);                      
        w = rng_next(MAX_WEIGHT) + 1;         
        add_edge(u, i, w);
    }

    while (edgeCount < E) {
        u = rng_next(V);
        v = rng_next(V);
        if (u == v) continue;                 
        if (weight[u][v] != 0) continue;      
        w = rng_next(MAX_WEIGHT) + 1;
        add_edge(u, v, w);
    }
}


static int dist[V][V];    
static int next_v[V][V];  

static void floyd_warshall(void) {
    int i, j, k;

    for (i = 0; i < V; i++) {
        for (j = 0; j < V; j++) {
            if (i == j) {
                dist[i][j] = 0;
                next_v[i][j] = i;
            }
            else if (weight[i][j] != 0) {
                dist[i][j] = weight[i][j];
                next_v[i][j] = j;            
            }
            else {
                dist[i][j] = INF;
                next_v[i][j] = -1;          
            }
        }
    }

    for (k = 0; k < V; k++) {
        for (i = 0; i < V; i++) {
            for (j = 0; j < V; j++) {
                if (dist[i][k] == INF || dist[k][j] == INF) continue;
                if (dist[i][k] + dist[k][j] < dist[i][j]) {
                    dist[i][j] = dist[i][k] + dist[k][j];
                    next_v[i][j] = next_v[i][k];   
                }
            }
        }
    }
}


static int build_path(int u, int v, int path[], int* sum) {
    int len = 0, cur = u;

    *sum = 0;
    if (next_v[u][v] == -1) return 0;         

    path[len++] = cur;
    while (cur != v) {
        int nxt = next_v[cur][v];
        *sum += weight[cur][nxt];             
        cur = nxt;
        path[len++] = cur;
    }
    return len;
}


static unsigned long ask_seed(void) {
    char line[64];
    unsigned long value = 0;
    int i = 0, digits = 0;

    printf("난수 시드를 입력하세요 (그냥 Enter를 누르면 %d): ", DEFAULT_SEED);
    if (fgets(line, sizeof(line), stdin) == NULL) return DEFAULT_SEED;
    while (line[i] == ' ' || line[i] == '\t') i++;
    while (line[i] >= '0' && line[i] <= '9') {
        value = value * 10 + (unsigned long)(line[i] - '0');
        digits++; i++;
    }
    return (digits > 0) ? value : DEFAULT_SEED;
}


int main(void) {
    int i, u, v, len, sum, pairCount = 0, mismatch = 0, unreachable = 0;
    int path[V];

    rng_seed(ask_seed());
    generate_graph();

    printf("\nGraph\n");
    printf("Vertices : %d\n", V);
    printf("Edges    : %d\n\n", edgeCount);

    printf("Edges\n");
    for (i = 0; i < edgeCount; i++) {
        printf("(%d, %d) Weight: %3d\n", edges[i].u, edges[i].v, edges[i].w);
    }

    floyd_warshall();

    printf("\nShortest Paths\n");
    for (u = 0; u < V; u++) {
        for (v = u + 1; v < V; v++) {          
            len = build_path(u, v, path, &sum);
            pairCount++;

            printf("\n%d -> %d\n", u, v);
            if (len == 0) {
                printf("Path     : (경로 없음)\n");
                unreachable++;
                continue;
            }
            printf("Path     : ");
            for (i = 0; i < len; i++) {
                printf("%d", path[i]);
                if (i < len - 1) printf(" -> ");
            }
            printf("\nDistance : %d\n", dist[u][v]);

            if (sum != dist[u][v]) mismatch++; 
        }
    }

    printf("\nTotal vertex pairs : %d\n", pairCount);

    printf("\n[확인 사항]\n");
    printf("  정점 수 10개            : %s\n", (V == 10) ? "예" : "아니오");
    printf("  간선 수 20개            : %s\n", (edgeCount == E) ? "예" : "아니오");
    printf("  자기 연결/중복 간선 없음 : 예 (생성 단계에서 차단)\n");
    printf("  연결 그래프             : %s\n", (unreachable == 0) ? "예 (모든 쌍에 경로 존재)" : "아니오");
    printf("  정점 쌍 45개 출력       : %s\n", (pairCount == 45) ? "예" : "아니오");
    printf("  경로의 가중치 합 = 최단거리 : %s\n",
        (mismatch == 0) ? "모든 쌍 일치" : "불일치 발생");
    return 0;
}