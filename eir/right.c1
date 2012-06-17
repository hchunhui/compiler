int heap[11] = {0,91,48,1,80,28,22,15,88,59,5};
int heap_length=11;
int heapSize = heap_length-1;
	
/*
     * @param i : 待交换元素的位置
     * @param j : 待交换元素的位置
     * 交换位置i和位置j的元素
     */    
void swap(int i,int j){
	int temp = heap[i];
	heap[i] = heap[j];
	heap[j] = temp;
}
	/*
     * @param i : 待建的子堆的根元素
     * 构建以i为根的子树为大根堆 
     */
void maxHeap(int i){
	int lc = 2*i;
	int rc = 2*i +1;
	int largest = i;
	int temp = 0;
	
	if(lc <= heapSize && heap[lc]>heap[i])
		largest = lc;
	else
		largest = i;
	
	if(rc <= heapSize && heap[rc] > heap[largest])
		largest = rc;
    
	if(largest != i){
		swap(i,largest);
			
		maxHeap(largest);
	}		
}
/*
     * 将整个数组构建成一个大根堆
     */
void buildMaxHeap(){
	int i = 0;
	i = heapSize/2;
	while(i>=1){
		maxHeap(i);
		i=i-1;
	}	
}

	 /*
     * 堆排序算法的主函数 
     */ 
void sort(){	
	int i=0;
	buildMaxHeap();
	i = heapSize;
	while(i>=2){
		swap(1,i);	
		heapSize=heapSize-1;
		maxHeap(1);
		i=i-1;
	}
}
    
    
    /*
     * 输出堆中的元素
     */
void printHeap(){
	int i = 1;
	while(i < heap_length){
		write(heap[i]);
		write("\t");
		i=i+1;
	}
	write("\n");
}

int main(){
	
	write("Before the sort\n");
	write("==================\n");
	printHeap();
	sort();
	write("After the sort\n");
	write("==================\n");
	printHeap();	
}