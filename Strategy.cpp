#include <iostream>#include <unistd.h>#include <cmath>#include <time.h>#include "Point.h"#include "Strategy.h"using namespace std;const int Max_node = 2000000;int used_node = 0;int tmpboard[20][20];int tmptop[20];int row = 0;int col = 0;int remaincol = 0;int mynoX;int mynoY;int vistime = 0;int starttime = 0;double time_bound = 2.6;void reset(const int* _board, const int* top){    vistime = 0;    used_node = 0;}void clean_board(const int* _board, const int* top){    for(int i=0; i<row; i++)        for(int j=0; j<col; j++)            tmpboard[i][j] = _board[i * col + j];    remaincol = 0;    for(int i=0; i<col; i++)    {        tmptop[i] = top[i];        if(top[i]!=0) remaincol++;    }}void add_piece(int pos, int team){#ifdef DEBUG    if(tmptop[pos] == 0) printf("add piece fault\n"); //【调试】#endif//    if(tmptop[pos] == 0) return;    tmpboard[tmptop[pos]-1][pos] = team;    if(pos==mynoY && tmptop[pos]-2 == mynoX) tmptop[pos] = mynoX;    else tmptop[pos]--;    if(tmptop[pos] == 0) remaincol--;}bool my_Win1(const int x, const int y);bool my_Win2(const int x, const int y);bool my_isTie();class Node{public:    int x = 0; //棋子放置的行数    int y = 0; //列数    int win;    int total;    int team;    double belief;    bool can_expand = true;    bool if_terminate = true;    int id;    int parent;    int children[12];    int children_num = 0;    void clear();    int expand();    int best_child();    int best_child_final();    void calc_belief();};Node nodes[Max_node];void Node::clear(){    children_num = 0;    win = 0;    total = 0;//    calc_belief();//可以不加    if_terminate = false;    if(remaincol != 0)        can_expand = true;    else        can_expand = false;    //没有子节点导致的terminate在Node::expand判断my_isTie时控制}void Node::calc_belief(){    double k = 1;    belief = win/double(total) + k*sqrt(2*log(nodes[parent].total)/total); //这里改了//    belief = win/double(total) + k*sqrt(2*log(total)/nodes[parent].total);}int Node::best_child(){    int res = 0;    double maxb = -1;    for (int i = 0; i < children_num; ++i) {        nodes[children[i]].calc_belief();//        printf("child %d belief = %f id=%d\n", i, nodes[children[i]].belief, children[i]);        if(nodes[children[i]].belief > maxb)        {            maxb = nodes[children[i]].belief;            res = children[i];        }    }    return res;}int Node::best_child_final(){    int res = children[0];    double maxs = -1.0;    for(int i=0; i<children_num; ++i)    {        if(nodes[children[i]].if_terminate)        {            res = children[i];            break;        }        //加入常识：不给下一步创造机会        bool ok = true;        for(int j=0; j<nodes[children[i]].children_num; j++)        {            if(nodes[nodes[children[i]].children[j]].if_terminate)            {                ok = false;                break;            }        }        if(!ok) continue;        double score = nodes[children[i]].win/(double)nodes[children[i]].total;        if(score > maxs)        {            maxs = score;            res = children[i];        }    }    return res;}int Node::expand(){    children[children_num] = used_node;    children_num++;    if(children_num == remaincol)    {        can_expand = false; //现在的remaincol还没有加上新放的子//        printf("id = %d can't expand\n", id);//        for(int i=0; i<children_num; i++)//            printf("child: %d\n", children[i]);    }    int startpos = (children_num==1) ? 0 : nodes[children[children_num-2]].y+1;#ifdef DEBUG    if(startpos < 0 || startpos >= col)    {        printf("!!! id=%d, children_num = %d, startpos = %d\n", id, children_num, startpos);        for(int i=0; i<children_num-1; i++) printf("%d ", children[i]);        printf("\n");    }#endif    while(tmptop[startpos] == 0) startpos++;    nodes[used_node].y = startpos;    nodes[used_node].x = tmptop[startpos]-1;    nodes[used_node].parent = id;    nodes[used_node].id = used_node;    nodes[used_node].team = 3-team;#ifdef DEBUG    //调试用    if(tmptop[startpos] <= 0)    {        printf("expand fault: tmptop[%d] = %d\n", startpos, tmptop[startpos]);        printf("children_num = %d, col = %d\n", children_num, col);    }#endif    add_piece(startpos, nodes[used_node].team); //已经改变了棋盘状态    nodes[used_node].clear(); //利用改变后的remain_col，才能正确计算能不能expand    if((nodes[used_node].team == 1 && my_Win1(nodes[used_node].x, nodes[used_node].y)) ||            (nodes[used_node].team == 2 && my_Win2(nodes[used_node].x, nodes[used_node].y)) ||            my_isTie())        nodes[used_node].if_terminate = true;    used_node++;    return used_node-1;}int tree_policy(int id){    while(!nodes[id].if_terminate)    {//        printf("tree\n");        if(nodes[id].can_expand)        {#ifdef DEBUG            printf("expand\n");            if(id!=0 && tmptop[nodes[id].y] <= 0)            {                printf("BEFORE expand fault: nodes[%d].y = %d, tmptop[nodes[id].y] = %d", id,nodes[id].y,tmptop[nodes[id].y] );            }#endif            if(id != 0) add_piece(nodes[id].y, nodes[id].team);            return nodes[id].expand();        }        else        {            //【调试用】#ifdef DEBUG            if((id != 0) && tmptop[nodes[id].y] <= 0)            {                printf("tree fault: nodes[%d].y = %d, tmptop[nodes[id].y] = %d", id,nodes[id].y,tmptop[nodes[id].y] );            }#endif            if(id != 0) add_piece(nodes[id].y, nodes[id].team);            id = nodes[id].best_child();//            printf("best child id = %d\n", id);        }    }    return id;}int default_policy(int id){    if(nodes[id].if_terminate)    {        return nodes[id].team;    }    int team = nodes[id].team;    while(true)    {//        printf("default in\n");        team = 3-team;        int rd = rand() % remaincol;        int nexty = 0;        while(tmptop[nexty]==0) nexty++;        for(int i=0; i<rd; i++)        {            nexty++;            while(tmptop[nexty]==0) nexty++;        }        //调试#ifdef DEBUG        if(nexty >= col)        {            printf("nexty fault\n");        }#endif        int nextx = tmptop[nexty]-1;#ifdef DEBUG        if(nextx < 0)        {            printf("nextx fault\n");            printf("tmptop[%d] = %d\n", nexty, tmptop[nexty]);        }#endif        add_piece(nexty, team);//        printf("finish add piece in\n");//        for(int i=0; i<col; i++) printf("%d ", tmptop[i]);//        printf("\n");//        printf("col:%d  remain:%d\n", col, remaincol);        if((team == 1 && my_Win1(nextx, nexty)) ||           (team == 2 && my_Win2(nextx, nexty))  || my_isTie())        {//            printf("finish judge win");            return team;        }//        printf("finish judge win");    }}void backup(int leaf, int team){    while(leaf != -1)    {        if(nodes[leaf].team == team)        {            nodes[leaf].win ++;        }        nodes[leaf].total++;//        nodes[leaf].calc_belief();        leaf = nodes[leaf].parent;    }}int UCTSearch(int root, const int* _board, const int* top){    reset(_board, top);    nodes[root].clear();    nodes[root].parent = -1;    nodes[root].team = 1;    nodes[root].can_expand = 1;    //肯定can_expand    used_node = root+1;    vistime = 0;    while(((clock()-starttime) < time_bound * CLOCKS_PER_SEC) && used_node < Max_node)    {//        printf("in\n");        vistime++;        clean_board(_board, top);//        printf("tree\n");        int leaf = tree_policy(root);//        printf("default\n");        int winteam = default_policy(leaf);//        printf("backup\n");        backup(leaf, winteam);//        printf("finish\n");    }//    for(int i=0; i<row; i++)//    {//        for(int j=0; j<col; j++)//            printf("%d ", tmpboard[i][j]);//        printf("\n");//    }//    printf("\n\n");//    for(int i=0; i<col; i++) printf("%d ", tmptop[i]);//    printf("\n");//    printf("out\n");    return nodes[nodes[root].best_child_final()].y;}/*	策略函数接口,该函数被对抗平台调用,每次传入当前状态,要求输出你的落子点,该落子点必须是一个符合游戏规则的落子点,不然对抗平台会直接认为你的程序有误		input:		为了防止对对抗平台维护的数据造成更改，所有传入的参数均为const属性		M, N : 棋盘大小 M - 行数 N - 列数 均从0开始计， 左上角为坐标原点，行用x标记，列用y标记		top : 当前棋盘每一列列顶的实际位置. e.g. 第i列为空,则_top[i] == M, 第i列已满,则_top[i] == 0		_board : 棋盘的一维数组表示, 为了方便使用，在该函数刚开始处，我们已经将其转化为了二维数组board				你只需直接使用board即可，左上角为坐标原点，数组从[0][0]开始计(不是[1][1])				board[x][y]表示第x行、第y列的点(从0开始计)				board[x][y] == 0/1/2 分别对应(x,y)处 无落子/有用户的子/有程序的子,不可落子点处的值也为0		lastX, lastY : 对方上一次落子的位置, 你可能不需要该参数，也可能需要的不仅仅是对方一步的				落子位置，这时你可以在自己的程序中记录对方连续多步的落子位置，这完全取决于你自己的策略		noX, noY : 棋盘上的不可落子点(注:涫嫡饫锔?龅膖op已经替你处理了不可落子点，也就是说如果某一步				所落的子的上面恰是不可落子点，那么UI工程中的代码就已经将该列的top值又进行了一次减一操作，				所以在你的代码中也可以根本不使用noX和noY这两个参数，完全认为top数组就是当前每列的顶部即可,				当然如果你想使用lastX,lastY参数，有可能就要同时考虑noX和noY了)		以上参数实际上包含了当前状态(M N _top _board)以及历史信息(lastX lastY),你要做的就是在这些信息下给出尽可能明智的落子点	output:		你的落子点Point*/extern "C" Point* getPoint(const int M, const int N, const int* top, const int* _board, 	const int lastX, const int lastY, const int noX, const int noY){    starttime = clock();	/*		不要更改这段代码	*/	int x = -1, y = -1;//最终将你的落子点存到x,y中	int** board = new int*[M];	for(int i = 0; i < M; i++){		board[i] = new int[N];		for(int j = 0; j < N; j++){			board[i][j] = _board[i * N + j];		}	}		/*		根据你自己的策略来返回落子点,也就是根据你的策略完成对x,y的赋值		该部分对参数使用没有限制，为了方便实现，你可以定义自己新的类、.h文件、.cpp文件	*/	//Add your own code below	row = M; col = N;	mynoX = noX; mynoY = noY;    srand(time(0));    y = UCTSearch(0, _board, top);    x = top[y]-1;    cerr << "x = " << x << "; y =" << y << "; used =" <<used_node << " vistime="<<vistime<<endl;//    printf("x = %d; y = %d; used = %d\n ", x, y, used_node);////////    for(int i=0; i<12; i++)//    {//        if(nodes[i].parent == 0)//            printf("id = %d, parent = %d, total = %d, win = %d, team = %d\n", i, nodes[i].parent, nodes[i].total, nodes[i].win, nodes[i].team);//    }//    for(int j=0; j<12; j++)//        for(int i=0; i<200; i++)//            if(nodes[i].parent == j)//                cerr << "id="<<nodes[i].id << " parent="<<nodes[i].parent<<" total="<<nodes[i].total<<" win="<<nodes[i].win<<endl;    /*        不要更改这段代码    */	clearArray(M, N, board);	return new Point(x, y);}/*	getPoint函数返回的Point指针是在本dll模块中声明的，为避免产生堆错误，应在外部调用本dll中的	函数来释放空间，而不应该在外部直接delete*/extern "C" void clearPoint(Point* p){	delete p;	return;}/*	清除top和board数组*/void clearArray(int M, int N, int** board){	for(int i = 0; i < M; i++){		delete[] board[i];	}	delete[] board;}/*	添加你自己的辅助函数，你可以声明自己的类、函数，添加新的.h .cpp文件来辅助实现你的想法*/bool my_Win1(const int x, const int y){    //横向检测    int i, j;    int count = 0;    for (i = y; i >= 0; i--)        if (!(tmpboard[x][i] == 1))            break;    count += (y - i);    for (i = y; i < col; i++)        if (!(tmpboard[x][i] == 1))            break;    count += (i - y - 1);    if (count >= 4) return true;    //纵向检测    count = 0;    for (i = x; i < row; i++)        if (!(tmpboard[i][y] == 1))            break;    count += (i - x);    if (count >= 4) return true;    //左下-右上    count = 0;    for (i = x, j = y; i < row && j >= 0; i++, j--)        if (!(tmpboard[i][j] == 1))            break;    count += (y - j);    for (i = x, j = y; i >= 0 && j < col; i--, j++)        if (!(tmpboard[i][j] == 1))            break;    count += (j - y - 1);    if (count >= 4) return true;    //左上-右下    count = 0;    for (i = x, j = y; i >= 0 && j >= 0; i--, j--)        if (!(tmpboard[i][j] == 1))            break;    count += (y - j);    for (i = x, j = y; i < row && j < col; i++, j++)        if (!(tmpboard[i][j] == 1))            break;    count += (j - y - 1);    if (count >= 4) return true;    return false;}bool my_Win2(const int x, const int y){    //横向检测    int i, j;    int count = 0;    for (i = y; i >= 0; i--)        if (!(tmpboard[x][i] == 2))            break;    count += (y - i);    for (i = y; i < col; i++)        if (!(tmpboard[x][i] == 2))            break;    count += (i - y - 1);    if (count >= 4) return true;    //纵向检测    count = 0;    for (i = x; i < row; i++)        if (!(tmpboard[i][y] == 2))            break;    count += (i - x);    if (count >= 4) return true;    //左下-右上    count = 0;    for (i = x, j = y; i < row && j >= 0; i++, j--)        if (!(tmpboard[i][j] == 2))            break;    count += (y - j);    for (i = x, j = y; i >= 0 && j < col; i--, j++)        if (!(tmpboard[i][j] == 2))            break;    count += (j - y - 1);    if (count >= 4) return true;    //左上-右下    count = 0;    for (i = x, j = y; i >= 0 && j >= 0; i--, j--)        if (!(tmpboard[i][j] == 2))            break;    count += (y - j);    for (i = x, j = y; i < row && j < col; i++, j++)        if (!(tmpboard[i][j] == 2))            break;    count += (j - y - 1);    if (count >= 4) return true;    return false;}bool my_isTie(){    return remaincol == 0;}