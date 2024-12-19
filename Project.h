#include <iostream>
#include <time.h>
#include <fstream>
#include<string>
#include "SHA1.h"
using namespace std;

//-----------------------------------------------------------------------------Class FileNode----------------------------------------------------------------------------------
class FileNode
{
	int hash;
	string value;
	FileNode* next;
	string name;
public:
	FileNode() {
		hash = -1;
		value = "";
		next = nullptr;
		name = "";
	}

	FileNode(int file_hash, string val, string naam) {
		hash = file_hash;
		value = val;
		name = naam;
		next = nullptr;
	}

	friend class BTreeNode;
	friend class MachineNode;
	friend class RingDHT;
};

//-----------------------------------------------------------------------------Class Routing Table Node----------------------------------------------------------------------------------
class RoutingTableNode {
	// To store pointer to some node in the Ring
	MachineNode* MachPtr;
	// To traverse the Doubly Linked List storing all Pointers
	RoutingTableNode* nextRT;
	RoutingTableNode* prevRT;
	int offset;
public:
	RoutingTableNode() : MachPtr(), nextRT(nullptr), prevRT(nullptr), offset(-1) {}
	RoutingTableNode(MachineNode*& M) : MachPtr(M), nextRT(nullptr), prevRT(nullptr), offset(-1) {}

	friend class RingDHT;
	friend class MachineNode;
};

//-----------------------------------------------------------------------------Class BTree Node--------------------------------------------------------------------------------------
class BTreeNode
{
	FileNode* keys;		//an array of keys
	int degree;			//minimum degree (range for number of keys)
	BTreeNode** Child;	//an array of child pointers
	int n;				//current number of keys
	bool leaf;			//is true when node is leaf
	int files;			//number of files stored on this node
public:
	BTreeNode() {
		degree = -1;
		n = -1;
		Child = nullptr;
		keys = nullptr;
		leaf = 0;
		files = 0;
	}
	BTreeNode(int trange, bool l) {
		degree = trange;
		leaf = l;
		keys = new FileNode[(2 * degree) - 1];
		Child = new BTreeNode * [2 * degree];
		n = 0;
		files = 0;
		cout << "BTreeNode created with n = " << 2 * degree << endl;
	}

	//Inserting element in a non-full node
	void insertInNode(int file_hash, string v, string naam);
	//splits child y of the full node
	void splitNode(int i, BTreeNode* y);
	//A function to traverse all nodes in a subtree rooted with this node.
	void traverse();
	//Function to search for file node.
	string search(int file_hash, string naam);
	//Function to use to delete a file node.
	void searchForDeletion(int file_hash, FileNode*& NodetoReturn);
	//Function to remove the file node.
	void remove(int k, string naam);
	//function to remove the idx-th key from this node - which is a leaf node
	void removeLeafNode(int idx, string naam);
	//function to remove the idx-th key from this node - which is a non-leaf node
	void removeNonLeafNode(int idx, string naam);
	//function to get predecessor of keys[idx]
	//function to get predecessor of keys[idx]
	FileNode Predecessor(int idx);
	//function to get successor of keys[idx]
	FileNode Successor(int idx);
	//function to fill child C[idx] which has less than t-1 keys.
	void FillNode(int idx);
	//Function to get from previous node.
	void GetFromPrevious(int idx);
	//Function to get from next node.
	void GetFromNext(int idx);
	//Function to merge.
	void MergeNodes(int idx);
	//Function to check collision in a node
	void checkCollision(int file_hash, string v, string naam, bool& collision, int& index);

	friend class MachineNode;
	friend class RingDHT;
};

void BTreeNode::splitNode(int i, BTreeNode* y)
{
	cout << "\nSplit Node:\n";
	//Create a new node which is going to store (t-1) keys of y
	BTreeNode* z = new BTreeNode(y->degree, y->leaf);
	z->n = degree - 1;

	//Copy the last (t-1) keys of y to z
	for (int j = 0; j < degree - 1; j++) {
		z->keys[j] = y->keys[j + degree];
	}

	//copy the last t children of y to z
	if (!y->leaf) {
		for (int j = 0; j < degree; j++) {
			z->Child[j] = y->Child[j + degree];
		}
	}
	//Reduce the number of keys in y
	y->n = degree - 1;
	//Since this node is going to have a new child,
	//create space for new child
	for (int j = n; j >= i + 1; j--) {
		Child[j + 1] = Child[j];
	}

	//Link the new child to this node
	Child[i + 1] = z;

	//A key of y will move to this node. Find the location of new key and move all greater keys one space ahead
	for (int j = n - 1; j >= i; j--) {
		keys[j + 1] = keys[j];
	}
	//Copy the middle key of y to this node
	keys[i] = y->keys[degree - 1];

	//Increment count of keys in this node
	n++;
}

void BTreeNode::traverse() {
	//There are n keys and n+1 children, traverse through n keys and first n children
	int i;
	for (i = 0; i < n; i++) {
		//if this is not leaf, then before printing key[i], traverse the subtree rooted with child C[i].
		if (leaf == false) {
			Child[i]->traverse();
		}
		cout << " " << keys[i].hash;
		if (keys[i].next != NULL) {
			FileNode* temp = keys[i].next;
			cout << " " << temp->hash;
			while (temp->next != NULL) {
				temp = temp->next;
				cout << " " << temp->hash;
			}
		}
	}

	//Print the subtree rooted with last child
	if (leaf == false) {
		Child[i]->traverse();
	}

}

string BTreeNode::search(int file_hash, string naam)
{
	//find the first key greater than or equal to k
	int i = 0;
	while (n > i && keys[i].hash < file_hash) {
		i++;
	}

	//if the found key is equal to k , return this node
	if (keys[i].hash == file_hash) {
		if (keys[i].name == naam) {	//if file found on the node 
			return keys[i].value;
		}
		else {	//traversing the chain if it exists
			if (keys[i].next != NULL) {
				FileNode* temp = keys[i].next;
				while (temp->next != NULL && temp->name != naam) {
					temp = temp->next;
				}
				if (temp->name == naam) {
					return temp->value;
				}
				else {
					cout << "File not found" << endl;
					return "";
				}
			}
		}
	}

	//If key is not found here and this is a leaf node
	if (leaf) {
		cout << "File not found" << endl;
		return "";
	}
	//Go to the appropriate child
	return Child[i]->search(file_hash, naam);
}

void BTreeNode::searchForDeletion(int file_hash, FileNode*& NodetoReturn)
{
	//find the first key greater than or equal to k
	int i = 0;
	while (n > i && keys[i].hash < file_hash) {
		i++;
	}

	//if the found key is equal to k , return this node
	if (keys[i].hash == file_hash) {
		NodetoReturn = &keys[i];
	}

	//If key is not found here and this is a leaf node
	if (leaf) {
		cout << "File not found" << endl;
		return;
	}
	//Go to the appropriate child
	Child[i]->searchForDeletion(file_hash, NodetoReturn);
}

void BTreeNode::remove(int k, string naam) {	//deal with chaining!! 
	int idx = 0;
	while (idx < n && keys[idx].hash < k) {
		idx++;
	}

	//key present in this node
	if (idx < n && keys[idx].hash == k) {
		if (keys[idx].next == nullptr)
		{

			if (leaf) {
				cout << "Remove from Leaf\n";
				removeLeafNode(idx, naam);
			}
			else {
				cout << "Remove from Non-Leaf\n";
				removeNonLeafNode(idx, naam);
			}
			return;
		}
		else if (keys[idx].name == naam)	//collision + removal of head
		{
			keys[idx] = *keys[idx].next;
			return;
		}
		else 								//collision + removal of link from chain
		{
			FileNode* temp = keys[idx].next;
			FileNode* prev = temp;
			while (temp != nullptr)
			{
				if (temp->name == naam)
				{
					if (prev != temp)
					{
						prev->next = temp->next;
					}
					else
					{
						keys[idx].next = temp->next;

					}
					return;
				}
				prev = temp;
				temp = temp->next;
			}
			if (temp == nullptr)
			{
				cout << "File not found" << endl;
				return;
			}
		}
	}
	else {
		//if this node is a leaf node, then the key is not present in tree
		if (leaf) {
			cout << "the key does not exist in this tree" << endl;
			return;
		}

		//the key to be removed is present in the subtree rooted with this node. The flag indicates whether the key is present in the sub-tree rooted with the last child of this node
		bool flag = 0;
		if (idx == n)
			flag = true;


		//if the child where the key is supposed to exist has less than t keys, we fill that child
		if (Child[idx]->n < degree) {
			FillNode(idx);
		}

		//If the last child has been merged, it must have merged with the previous child and so we recurse on the (idx)th child which now has atleast t keys
		if (flag && idx > n) {
			Child[idx - 1]->remove(k, naam);
		}
		else {
			Child[idx]->remove(k, naam);
		}
		return;
	}
}

void BTreeNode::removeLeafNode(int idx, string naam) {
	if (keys[idx].name == naam)
	{
		if (keys[idx].next == nullptr)
		{
			//Move all the keys after the idx-th pos one place backward
			for (int i = idx + 1; i < n; i++)
			{
				keys[i - 1] = keys[i];
			}
			n--;
			files--;
		}
		else
		{
			FileNode* temp = keys[idx].next;
			keys[idx] = *temp;
		}
	}
	else
	{
		if (keys[idx].next != nullptr)
		{
			FileNode* temp = keys[idx].next;
			FileNode* prev = nullptr;
			while (temp != nullptr && temp->name != naam)
			{
				prev = temp;
				temp = temp->next;
			}
			if (temp != nullptr)
			{
				if (prev != nullptr)
				{
					prev->next = temp->next;
					//temp = nullptr;
					files--;
				}
				else
				{
					//temp = nullptr;
					keys[idx].next = nullptr;
					files--;
				}
			}
			else
			{
				cout << "File not found" << endl;
			}
		}
	}
	//reduce the count of keys
}

void BTreeNode::removeNonLeafNode(int idx, string naam)
{
	int k = keys[idx].hash;


	//if the child that precedes k (C[idx]) has atleast t keys, find the predecessor 'pred' of k in the subtree rooted at C[idx]. Replace k by pred. Recursively delete pred in C[idx]

	if (Child[idx]->n >= degree) {
		FileNode pred = Predecessor(idx);
		keys[idx] = pred;
		Child[idx]->remove(pred.hash, pred.name);
	}

	//if the child C[idx] has less than t keys, examine C[idx+1].
	//If C[idx+1] has atleast t keys, find the usccessor 'succ' of k in the subtree rooted at C[idx+1] 
	//Replace k by succ
	//recursively delete succ in C[idx+1]

	else if (Child[idx + 1]->n >= degree) {
		FileNode succ = Successor(idx);
		keys[idx] = succ;
		Child[idx + 1]->remove(succ.hash, succ.name);
	}

	//IF both Child[idx] and Child[idx+1] have less than t keys, merge k and all of Child[idx+1] into Child[idx]
	//Now Child[idx] containts 2t-1 keys
	//Free Child[idx+1] and recursively delete k from Child[idx]
	else {
		MergeNodes(idx);
		Child[idx]->remove(k, naam);
	}
	return;
}

FileNode BTreeNode::Predecessor(int idx) {
	//Keep moving to the right most node until we  reach a leaf
	BTreeNode* curr = Child[idx];
	while (!curr->leaf) {
		curr = curr->Child[curr->n];
	}
	//return the last key
	return curr->keys[curr->n - 1];
}

FileNode BTreeNode::Successor(int idx) {
	//Keep moving to the eft most node starting from CHild[idx+1] until we reach a leaf
	BTreeNode* curr = Child[idx + 1];
	while (!curr->leaf) {
		curr = curr->Child[0];
	}
	return curr->keys[0];
}

void BTreeNode::FillNode(int idx) {
	cout << "Filling " << idx << endl;
	//if the previous child C[idx-1] has more than t-1 keys, borrow a key from that child
	if (idx != 0 && Child[idx - 1]->n >= degree) {
		GetFromPrevious(idx);
	}

	//if the next child (C[idx+1]) as more than t-1 keys, borrow a key from that child
	else if (idx != n && Child[idx + 1]->n >= degree) {
		GetFromNext(idx);
	}
	//Merge Child[idx] with its sibling
	//if C[idx] is the last child, merge it with its previous sibling
	//Otherwise merge it with its next sibling
	else {
		if (idx != n) {
			MergeNodes(idx);
		}
		else {
			MergeNodes(idx - 1);
		}
	}
	return;
}

void BTreeNode::GetFromPrevious(int idx)
{
	cout << "Borrowing from Previous" << endl;
	BTreeNode* child = Child[idx];
	BTreeNode* sibling = Child[idx - 1];

	// The last key from C[idx-1] goes up to the parent and key[idx-1]
	// from parent is inserted as the first key in C[idx]. Thus, the sibling loses
	// one key and child gains one key

	//Moving ll keys in Child[idx] one step ahead
	for (int i = child->n - 1; i >= 0; i--) {
		child->keys[i + 1] = child->keys[i];
	}

	//IF C[idx] is not a leaf, move all its child pointers one step ahead
	if (!child->leaf) {
		for (int i = child->n; i >= 0; i--)
		{
			child->Child[i + 1] = child->Child[i];
		}
	}

	//Setting child's first key equal to eys[idx-1] from the current node
	child->keys[0] = keys[idx - 1];

	//Moving sibling's last child as Child[idx]'s first child
	if (!child->leaf) {
		child->Child[0] = sibling->Child[sibling->n];
	}

	//moving the key from the sbiling to the parent, this reduces the number of keys in the sibling
	keys[idx - 1] = sibling->keys[sibling->n - 1];
	child->n += 1;
	sibling->n -= 1;
	return;
}

void BTreeNode::GetFromNext(int idx)
{
	cout << "Borrowing from next" << endl;
	BTreeNode* child = Child[idx];
	BTreeNode* sibling = Child[idx + 1];

	//keys[idx] is inserted as the last key in Child[idx]
	int number = child->n;
	child->keys[number] = keys[idx];

	//Sibling's first child s inserted as the last childinto C[idx]
	if (!child->leaf) {
		child->Child[(child->n) + 1] = sibling->Child[0];
	}

	//the first key from sibling is inserted into keys[idx]
	keys[idx] = sibling->keys[0];

	//moving all keys in sibling one step behind
	for (int i = 1; i < sibling->n; i++) {
		sibling->keys[i - 1] = sibling->keys[i];
	}

	//Moving the child pointers one step behind
	if (!sibling->leaf) {
		for (int i = 1; i <= sibling->n; i++) {
			sibling->Child[i - 1] = sibling->Child[i];
		}
	}

	//Increasing and decreasing the count of Child[idx]and Child[idx+1] respectively
	child->n += 1;
	sibling->n -= 1;

	return;
}

void BTreeNode::MergeNodes(int idx)
{
	cout << "Merge\n";
	BTreeNode* child = Child[idx];
	BTreeNode* sibling = Child[idx + 1];

	//Pulling a key from the current node and inserting t into (t-1)th position of Child[idx]
	child->keys[degree - 1] = keys[idx];

	//Copying the keys from Child[idx+1] to Child[idx] at the end
	for (int i = 0; i < sibling->n; i++)
		child->keys[i + degree] = sibling->keys[i];

	//Copying the child pointers from Child[idx+1] to Child[idx]
	if (!child->leaf) {
		for (int i = 0; i <= sibling->n; i++)
			child->Child[i + degree] = sibling->Child[i];
	}

	//Moving all keys after idx in the current node one step before to fill the gap created by moving keys [idx] to child[idx]
	for (int i = idx + 1; i < n; i++)
		keys[i - 1] = keys[i];

	//Moving the child pointers after (idx+1) in the current node one step before
	for (int i = idx + 2; i <= n; i++) {
		Child[i - 1] = Child[i];
	}

	//Updating the count of child and the current node
	child->n += sibling->n + 1;
	n--;

	//Freeing the memory occupied by sibling
	delete(sibling);
	return;
}
void BTreeNode::checkCollision(int file_hash, string v, string naam, bool& collision, int& index)
{
	for (int i = 0; i < n; i++)
	{
		if (keys[i].hash == file_hash)
		{
			collision = 1;
			index = i;
		}
	}
}
void BTreeNode::insertInNode(int file_hash, string v, string naam)
{
	cout << "\nInsert in Node:\n";
	int i = n - 1;		//initialize index as index of rightmost element
	if (leaf) {
		bool collision = 0;
		int index = -1;
		checkCollision(file_hash, v, naam, collision, index);
		if (collision)
		{
			if (!keys[index].next) {
				FileNode* temp = new FileNode(file_hash, v, naam);
				keys[index].next = temp;
				files++;
			}
			else {
				FileNode* temp = keys[index].next;
				while (temp->next != NULL)
				{
					temp = temp->next;
				}
				temp->next = new FileNode(file_hash, v, naam);
				files++;
			}
			return;
		}
		//find location of new key to be inserted
		// move all greater keys ahead
		while (i >= 0 && keys[i].hash > file_hash) {
			keys[i + 1] = keys[i];
			i--;
		}

		FileNode temp(file_hash, v, naam);
		keys[i + 1] = temp;
		n++;
		files++;

	}
	else {
		bool collision = 0;
		int index = 1;
		checkCollision(file_hash, v, naam, collision, index);
		if (collision)
		{

			if (!keys[index].next) {
				FileNode* temp = new FileNode(file_hash, v, naam);
				keys[index].next = temp;
				files++;
			}
			else {
				FileNode* temp = keys[index].next;
				while (temp->next != NULL)
				{
					temp = temp->next;
				}
				temp->next = new FileNode(file_hash, v, naam);
				files++;
			}
			return;
		}
		//if this node is not leaf
		//find the child which is going to have the new key
		while (i >= 0 && keys[i].hash > file_hash) {
			i--;
		}

		//see if the found child is full
		if (Child[i + 1]->n == (2 * degree) - 1) {
			//if the child is full then split it
			splitNode(i + 1, Child[i + 1]);
			//after split, the middle key of C[i] goes up and C[i] is splitted into two. 
			//See which of the two is going to have the new key
			if (keys[i + 1].hash < file_hash) {
				i++;
			}
		}

		Child[i + 1]->insertInNode(file_hash, v, naam);

	}
}

//-----------------------------------------------------------------------------Class MachineNode--------------------------------------------------------------------------------------
class MachineNode
{
private:
	int machineID;
	MachineNode* next;
	// Doubly Linked List storing pointers to jump to other nodes
	RoutingTableNode* headRT;
	//Minimum range of a machine node.
	int minRange;
	// Number of Bits specified
	int identifierSpace;
	//B-tree root.
	BTreeNode* root;
	int t;

public:
	//Constructor.
	MachineNode() : minRange(-1), machineID(-1), next(nullptr), root(nullptr), identifierSpace(-1), headRT(nullptr) {}
	// Copy Constructor
	MachineNode(MachineNode*& M) : minRange(M->minRange), machineID(M->machineID), next(M->next), root(M->root), identifierSpace(M->identifierSpace), t(M->t) {}
	//Overloaded constructor.
	MachineNode(int idSpace, int machine_id) : minRange(-1), machineID(machine_id), next(nullptr), root(nullptr), identifierSpace(idSpace) {
		t = identifierSpace / 4;
		if (identifierSpace <= 15) {
			t = 2;
		}
		//cout << "Created a BTree with n = " << 2 * t << endl;
	}
	//To print the routing table.
	void PrintRoutingTable();
	//To change the order of key.
	void modifyKeys(int k);
	//A search function for deletion of file Node.
	void searchForDeletion(int file_hash, FileNode*& NodetoReturn);
	//Function to traverse the b-tree.
	void traverse();
	//Search function for searching a file.
	string search(int file_hash, string naam);
	//Function to insert the file node.
	void insert(int file_hash, string path, string naam);
	//Function to remove the file node.
	void remove(int k, string naam);
	void checkCollisions2(int file_hash, string v, string naam, bool& collision, int& index, int& kid);

	friend class RingDHT;
};

void MachineNode::remove(int k, string naam)
{
	if (!root) {
		cout << "The tree is empty" << endl;
		return;
	}
	else {
		root->remove(k, naam);

		//if the root node has 0 keys, make its first child as the new root if it has a child, else set root as null
		if (root->n == 0) {
			BTreeNode* temp = root;
			if (root->leaf) {
				root = NULL;
			}
			else {
				root = root->Child[0];
			}

			delete temp;
		}
	}

	return;
}
void MachineNode::checkCollisions2(int file_hash, string v, string naam, bool& collision, int& index, int& kid)
{
	for (int i = 0; i < root->n; i++)
	{

		if (root->keys[i].hash == file_hash)
		{
			collision = 1;
			//index = j;
			kid = i;
		}

	}
}

void MachineNode::insert(int file_hash, string v, string naam)
{
	cout << "Inserting node with hash = " << file_hash << endl;
	//If tree is empty
	if (!root) {
		root = new BTreeNode(t, true);
		FileNode temp(file_hash, v, naam);
		root->keys[0] = temp;
		root->n = 1;
		root->files++;
	}
	else {
		bool collision = 0;
		int index = -1;
		int kid = -1;
		checkCollisions2(file_hash, v, naam, collision, index, kid);
		if (collision)
		{
			root->insertInNode(file_hash, v, naam);
			return;
		}

		//If root is full then tree grows in height
		if (root->n == 2 * t - 1) {
			//Allocate memory for new root
			BTreeNode* s = new BTreeNode(t, false);

			//Make old root as child of new root
			s->Child[0] = root;

			//Split the old root & move 1 key to the new root
			s->splitNode(0, root);

			//New root has two children now. Decide which of the two children is going to have the new key
			int i = 0;
			if (s->keys[0].hash < file_hash) {
				i++;
			}

			s->Child[i]->insertInNode(file_hash, v, naam);

			//Change root
			root = s;
		}
		else { //if root is not full, call insertNonFull for root
			root->insertInNode(file_hash, v, naam);
		}


	}
}

void MachineNode::searchForDeletion(int file_hash, FileNode*& NodetoReturn)
{
	cout << "Searching " << file_hash << endl;
	if (!root) {
		return;
	}
	root->searchForDeletion(file_hash, NodetoReturn);
}

string MachineNode::search(int file_hash, string naam) {
	cout << "Searching \'" << file_hash << "\'" << endl;
	if (!root) {
		cout << "Root is null" << endl;
		return "";
	}
	string answer = root->search(file_hash, naam);
	return answer;
}

void MachineNode::traverse() {
	if (root) {
		root->traverse();
	}
}

void MachineNode::modifyKeys(int k)
{
	if (k == 1) {
		cout << "Error, can't make a tree with 0 keys" << endl;
		return;
	}
	else {
		if (k % 2 == 0) {
			t = k / 2;
		}
		else {
			t = k / 2;
			t++;
		}
		cout << "Modified the BTree to be with n = " << 2 * t << endl;
	}
}

void MachineNode::PrintRoutingTable() {
	RoutingTableNode* currNodeRT = headRT;
	cout << "Machine ID: " << machineID << endl;
	cout << "This Machine has Pointers to the following Nodes: \n";
	for (int i = 1; i <= identifierSpace; i++) {
		cout << i << ")" << currNodeRT->MachPtr->machineID << endl;
		currNodeRT = currNodeRT->nextRT;
	}
}

//-----------------------------------------------------------------------------Class RING DHT--------------------------------------------------------------------------------------
class RingDHT {
private:
	MachineNode* headMachine;
	MachineNode* tailMachine;
	int numberOf_Machines;

	int insertedMachines;

	int identifierValue;

	//------------------Functions------------------------
	//Function to set the minimum range for all the nodes.
	void setMinimumRange();

	//Creates RoutingTable For all Nodes
	void CreateRoutingTable();
	//Finds the next successor according to the formula provided - helper
	MachineNode*& findSuccessor(int IndexAdd);
	// Returns the head of every Routing Table for connection - helper
	RoutingTableNode*& Return_Routing_Head(RoutingTableNode*&);
	//Search for a Machine - helper
	MachineNode* SearchMachine(int, int);
	//Search for machine.
	MachineNode* SearchMachine(MachineNode*, int);
	// Prints Search Pathway
	void PrintPath(MachineNode*& currNode, bool ending);

	//Insert file helper.
	void InsertFile(int, int, string, string);
	//To delete the routing table of given machine.
	void DeleteRoutingTable(MachineNode*);
public:
	//For Circular SIngly Linked List.
	RingDHT(int, int);
	//Function to manually assign the machine id.
	void insertWith_ID(int, int);
	//Function to automatically assign the machine id.
	void insertID_automatically();
	//Function to insert data with machine name.
	void insertWith_MachineName(const char*);
	//to set ranges for b trees
	void setMinimumRangemimic();
	//Print Routing Table of any Node
	void PrintRouteTable(int);

	//To insert file into the system.
	void InsertFile(string, string, int);
	//To remove the specified file,
	void removeFile(int, int, string);
	//To search for the specified file.
	void searchFile(int, int, string);
	//To traverse the tree to find inserted node.
	void traverseTree(int, int);
	//Function to insert machine on the fly without disrupting the functionality of ring dht.
	void insert_Machine(int);
	//Function to insert machine on the fly without disrupting the functionality of ring dht.
	void insert_Machine(const char*);
	//Function to delete machine on the fly without disrupting the functionality of ring dht.
	void delete_Machine(int);

};

//Constructor of the main class.
RingDHT::RingDHT(int idValue, int numMachines) : headMachine(nullptr), numberOf_Machines(numMachines), identifierValue(idValue), insertedMachines(0) {
	long int maximumSpace = pow(2, identifierValue);
	if (numberOf_Machines >= maximumSpace) {
		cout << "ERROR! Number of machines can't be greater than identifier space.\n";
	}
	else {
		//Code Segment to Initialize the Number of Machines.
		MachineNode* currNode = headMachine;
		for (int index = 1; index <= numberOf_Machines; index++) {
			//Initialize the head of machine.
			if (!headMachine) {
				headMachine = new MachineNode(identifierValue, -1);
				currNode = headMachine;
			}
			//Initialize the next nodes of singly circular linked list.
			else {
				while (currNode->next) {
					currNode = currNode->next;
				}
				currNode->next = new MachineNode(identifierValue, -1);

				if (index == numberOf_Machines) {
					tailMachine = currNode->next;
				}
			}
		}
		if (!headMachine->next) {
			tailMachine = headMachine;
			headMachine->next = headMachine;
		}
		else {
			//Instead of catering the circular case above, we do it directly here
			//and join the last node with the head node to make it circular.
			currNode->next->next = headMachine;
		}
	}
}
//Making of the Circular Linked List with user input IDs.
void RingDHT::insertWith_ID(int ID, int insertAtMachine) {
	cout << "--------------------------Machine Insertion with Manual ID " << ID << " ----------------------------- " << endl;
	long long int range = pow(2, identifierValue);
	if (ID >= range) {
		cout << "The ID cannot be greater than identifier space.\n";
		return;
	}

	if (insertedMachines >= numberOf_Machines) {
		cout << "No more machines can be added.\n";
		return;
	}

	MachineNode* currNode = headMachine;	int index = 1;
	//Traverses the circular linked list till the index.
	while (index != insertAtMachine) {
		currNode = currNode->next;		index++;
	}
	//Puts the ID on machine.
	if (currNode->machineID == -1) {
		currNode->machineID = ID;
	}
	else {
		cout << "A machine already exists at this index.\n";
		return;
	}

	insertedMachines++;
	if (insertedMachines == numberOf_Machines)
		setMinimumRangemimic();
}

long long int convert_HexToDec(const char* hexArr, int numBits) {
	int numDigits = 0;
	//First we determine the number of hex values to be used.
	for (int multiple = 1; multiple <= 15; multiple++) {
		if (4 * multiple >= numBits) {
			numDigits = multiple;
			break;
		}
	}

	//Now to convert the hexadecimal number to decimal.
	int convertedNum = 0, len = 0;
	unsigned long long int convertedDecimal = 0;
	for (int index = 39, num_ofDigits = 1; num_ofDigits <= numDigits; num_ofDigits++, index--)
	{
		if (hexArr[index] >= '0' && hexArr[index] <= '9')
			convertedNum = hexArr[index] - 48;
		else if (hexArr[index] >= 'a' && hexArr[index] <= 'f')
			convertedNum = hexArr[index] - 87;
		else if (hexArr[index] >= 'A' && hexArr[index] <= 'F')
			convertedNum = hexArr[index] - 55;
		convertedDecimal += convertedNum * pow(16, len);
		len++;
	}

	return convertedDecimal;
}

long long int bitExtraction(long long int numToExtract, int numBits) {
	//To extract only number of identifier value amount bits.
	long long int bit_mask = pow(2, numBits) - 1;
	return numToExtract & bit_mask;
}

long long int hashFunction(const char* machineName, int identifierValue) {
	char result[21], hexresult[41];
	//To calculate the hash.
	SHA1(result, machineName, strlen(machineName));
	//Converting the result into hexadecimal.
	for (size_t offset = 0; offset < 20; offset++) {
		sprintf((hexresult + (2 * offset)), "%02x", result[offset] & 0xff);
	}

	long long int num = convert_HexToDec(hexresult, identifierValue);
	return bitExtraction(num, identifierValue);
}

void RingDHT::insertWith_MachineName(const char* machineName) {
	cout << "--------------------------Machine Insertion with MachineName-----------------------------" << endl;
	if (insertedMachines >= numberOf_Machines) {
		cout << "No more machines can be added.\n";
		return;
	}

	long long int hashed_integer = hashFunction(machineName, identifierValue);
	bool machineInserted = false;

	MachineNode* currNode = headMachine;
	for (int numMachine = 1; numMachine <= numberOf_Machines; numMachine++) {
		if (currNode->machineID == hashed_integer) {
			cout << "Machine with this ID " << hashed_integer << " already exists.\n";
			return;
		}

		if (currNode->machineID == -1) {
			if (currNode == tailMachine) {
				currNode->machineID = hashed_integer;
				machineInserted = true;
				break;
			}
			else if (currNode->next->machineID > hashed_integer) {
				currNode->machineID = hashed_integer;
				machineInserted = true;
				break;
			}
		}


		if (currNode->machineID > hashed_integer) {
			break;
		}

		currNode = currNode->next;

	}

	//We'll insert a new node at the appropriate place while deleting a node with id = -1.
	if (!machineInserted) {
		MachineNode* insertNode = new MachineNode(identifierValue, hashed_integer);
		//To reach the node where the new node is to be inserted.
		MachineNode* traversalNode = headMachine;
		while (traversalNode->next->machineID < hashed_integer && traversalNode->next != headMachine) {
			traversalNode = traversalNode->next;
		}

		if (traversalNode == headMachine) {
			insertNode->next = headMachine;
			tailMachine->next = headMachine = insertNode;
		}
		else {
			insertNode->next = traversalNode->next;
			traversalNode->next = insertNode;

			if (traversalNode == tailMachine) {
				tailMachine = insertNode;
			}
		}

		//Now we delete a machineNode with id = -1.
		while (traversalNode->next->machineID != -1) {
			traversalNode = traversalNode->next;
		}

		if (traversalNode->next == headMachine) {
			traversalNode->next = traversalNode->next->next;
			tailMachine->next = headMachine = traversalNode->next;
		}
		else {
			if (traversalNode->next == tailMachine) {
				tailMachine = traversalNode;
			}
			traversalNode->next = traversalNode->next->next;
		}
	}
	insertedMachines++;
	if (insertedMachines == numberOf_Machines)
		setMinimumRangemimic();
}

void RingDHT::insertID_automatically() {
	cout << "--------------------------Machine Insertion with Automatic ID-----------------------------" << endl;
	srand(time(0));
	long long int range = pow(2, identifierValue);
	int automatic_id = rand() % range;

	while (insertedMachines < numberOf_Machines) {
		//To check in the case a duplicate id arises.
		MachineNode* checkDups = headMachine;		bool isDup = false;
		while (true) {
			while (!isDup && checkDups->next != headMachine) {
				while (automatic_id == checkDups->machineID) {
					automatic_id = rand() % range;
					isDup = true;
				}
				checkDups = checkDups->next;
			}
			if (!isDup)
				break;
			checkDups = headMachine;	isDup = false;
		}

		//This will traverse the node to insert it at appropriate position.
		MachineNode* insertNode = new MachineNode(identifierValue, automatic_id);
		MachineNode* traversalNode = headMachine;

		if (automatic_id > headMachine->machineID) {
			while (traversalNode->next->machineID < automatic_id && traversalNode->next != headMachine) {
				traversalNode = traversalNode->next;
			}
		}

		if (traversalNode == headMachine) {
			insertNode->next = headMachine;
			tailMachine->next = headMachine = insertNode;
		}
		else {
			insertNode->next = traversalNode->next;
			traversalNode->next = insertNode;

			if (traversalNode == tailMachine) {
				tailMachine = insertNode;
			}
		}

		//Now we delete a machineNode with id = -1.
		while (traversalNode->next->machineID != -1) {
			traversalNode = traversalNode->next;
		}

		if (traversalNode->next == headMachine) {
			traversalNode->next = traversalNode->next->next;
			tailMachine->next = headMachine = traversalNode->next;
		}
		else {
			if (traversalNode->next == tailMachine) {
				tailMachine = traversalNode;
			}
			traversalNode->next = traversalNode->next->next;
		}

		insertedMachines++;
		if (insertedMachines == numberOf_Machines)
			setMinimumRangemimic();
		automatic_id = rand() % range;
	}
}

void RingDHT::setMinimumRangemimic() {
	MachineNode* currNode = headMachine;
	MachineNode* nextNode = headMachine->next;

	//Sets minimum range for all the machine nodes.
	while (currNode->next != headMachine) {
		nextNode->minRange = currNode->machineID + 1;

		if ((currNode->machineID - currNode->minRange) <= 15)
		{
			currNode->t = 2;
		}
		else
		{
			currNode->t = ((currNode->machineID) - (currNode->minRange)) / 2;
		}
		nextNode = nextNode->next;		currNode = currNode->next;
	}
	if (headMachine->next == headMachine) {
		headMachine->minRange = 0;
	}
	else {
		//To set 'Minimum Range' for HeadMachine.
		nextNode->minRange = currNode->machineID + 1;
		long long int range = pow(2, identifierValue);
		nextNode->minRange %= range;
	}
	long long int temp = pow(2, identifierValue);
	if (temp - currNode->machineID + nextNode->machineID <= 15)
	{
		nextNode->t = 4;
	}
	else
	{
		nextNode->t = (temp - currNode->machineID + nextNode->machineID) / 2;
	}
}

void RingDHT::setMinimumRange() {
	MachineNode* currNode = headMachine;
	MachineNode* nextNode = headMachine->next;

	//Sets minimum range for all the machine nodes.
	while (currNode->next != headMachine) {
		nextNode->minRange = currNode->machineID + 1;
		nextNode = nextNode->next;		currNode = currNode->next;
	}
	//To set 'Minimum Range' for HeadMachine.
	nextNode->minRange = currNode->machineID + 1;
	long long int range = pow(2, identifierValue);
	nextNode->minRange %= range;
}
//For the creation of Routing Table.
RoutingTableNode*& RingDHT::Return_Routing_Head(RoutingTableNode*& currNodeRT) {
	// To return the head of a Routing Table
	while (currNodeRT->prevRT) {
		currNodeRT = currNodeRT->prevRT;
	}
	return currNodeRT;
}

void RingDHT::CreateRoutingTable() {

	if (insertedMachines < numberOf_Machines) {
		insertID_automatically();
	}
	setMinimumRange();


	long int maximumSpace = pow(2, identifierValue);
	int indexPower = -1; // power of 2 = -1,1,2,3...IdentifierValue
	int RoutingRow = 0; // Number of entries per Table = IdentifierValue 
	long int indexAdd = pow(2, indexPower);
	int TraversalTime = 0;

	MachineNode* currNode = headMachine;

	while (TraversalTime < numberOf_Machines) {
		TraversalTime++;
		while (RoutingRow != identifierValue) {
			indexPower++; indexAdd = currNode->machineID + pow(2, indexPower); RoutingRow++;
			RoutingTableNode* currNodeRT = currNode->headRT;
			RoutingTableNode* prevNodeRT = nullptr;
			// Make Doubly Linked List and add CircularListPointers to every Node simultaneously 
			if (currNodeRT == nullptr) {
				MachineNode* addToRouteTable = findSuccessor(indexAdd);
				currNodeRT = new RoutingTableNode(addToRouteTable);

				if (currNode->machineID >= addToRouteTable->machineID) {
					currNodeRT->offset = addToRouteTable->machineID + maximumSpace;
				}
				else currNodeRT->offset = addToRouteTable->machineID;
			}
			else { //Append List to Add nodes
				while (currNodeRT) {
					prevNodeRT = currNodeRT;
					currNodeRT = currNodeRT->nextRT;
				}
				if (!currNodeRT) {
					MachineNode* addToRouteTable = findSuccessor(indexAdd);
					currNodeRT = new RoutingTableNode(addToRouteTable);

					if (currNode->machineID >= addToRouteTable->machineID) {
						currNodeRT->offset = addToRouteTable->machineID + maximumSpace;
					}
					else currNodeRT->offset = addToRouteTable->machineID;

					prevNodeRT->nextRT = currNodeRT;
					currNodeRT->prevRT = prevNodeRT;

				}
			}
			currNode->headRT = Return_Routing_Head(currNodeRT);
		}
		currNode = currNode->next; // Move to Routing Table of next node in CircularList
		RoutingRow = 0;  //reset rows
		indexPower = -1; //reset power
	}
}

MachineNode*& RingDHT::findSuccessor(int IndexAdd) {
	//Rounds up value to nearest (larger) node available in CircularList
	MachineNode* currNode = headMachine;
	long int maximumSpace = pow(2, identifierValue);
	long int FindingNumber = IndexAdd % maximumSpace;

	while (currNode) {
		if (FindingNumber > currNode->machineID && FindingNumber <= currNode->next->machineID) {
			return currNode->next;	//If Node found
		}
		else if (currNode->next == headMachine) {
			if (IndexAdd > currNode->machineID) {  //Between maximumSpace and 0
				if (IndexAdd <= maximumSpace) {
					return headMachine;
				}
				else if (IndexAdd > maximumSpace) { //After 0, till Node itself
					if (FindingNumber > headMachine->machineID) {
						currNode = currNode->next;
					}
					else {
						return headMachine;
					}
				}
			}
		}
		else {
			currNode = currNode->next; //Node not between current range, go to Next Node
		}
	}
}

//For the search of machine through Routing Table.
MachineNode* RingDHT::SearchMachine(int currPos, int key) {
	MachineNode* StartNode = this->SearchMachine(headMachine, currPos); // get current node
	return this->SearchMachine(StartNode, key); //find from current node
}

void RingDHT::PrintPath(MachineNode*& currNode, bool ending) {
	cout << currNode->machineID;
	if (!ending) cout << "->";
	else cout << endl;
}

MachineNode* RingDHT::SearchMachine(MachineNode* StartNode, int key) {
	long long int maximumSpace = pow(2, identifierValue);

	if (key > maximumSpace) {
		cout << "ERROR! Key of file can't be greater than identifier space.\n";
		exit(0); // Program Ends if input has an error
		return NULL;
	}

	if (!headMachine->headRT) {
		CreateRoutingTable();
	}

	//Searching must begin from a random Node everytime
	MachineNode* currNode = StartNode;
	cout << "searching:" << currNode->machineID << endl;

	MachineNode* prevNode = nullptr;
	MachineNode* prevprevNode = nullptr;
	bool MachineFound = false;

	PrintPath(currNode, 0);
	while (!MachineFound) {

		//Range found 
		if (currNode->machineID == (key % maximumSpace) || (currNode->machineID > (key % maximumSpace) && (key % maximumSpace) >= currNode->minRange)) {
			PrintPath(currNode, 1);
			MachineFound = true;
			break;
		}
		//To check Range of Head
		//Between current hash and 0
		if (currNode == headMachine && (key % maximumSpace) >= 0 && key < currNode->machineID) {
			PrintPath(currNode, 1);
			MachineFound = true;
			break;
		}
		//Between minRange of current(head) hash and maximumSpace
		else if (currNode == headMachine && (key % maximumSpace) >= currNode->minRange && (key % maximumSpace) < maximumSpace) {
			PrintPath(currNode, 1);
			MachineFound = true;
			break;
		}
		else { //Applying Algorithm 
			RoutingTableNode* currNodeRT = currNode->headRT;
			RoutingTableNode* prevNodeRT = nullptr;
			//currentMachineID < key and key <= currentMachine, first RoutingTableEntry - search forwarded to first RoutingTableEntry  
			if (currNode->machineID < key && key <= currNodeRT->offset) {
				prevprevNode = prevNode;
				prevNode = currNode;
				currNode = currNode->headRT->MachPtr;
				PrintPath(currNode, 0);
				continue;
			}
			else {
				//Find Between whichever Pointers in RoutingTable the Node to search lies in
				//Return the first of the two
				prevNodeRT = currNodeRT;
				currNodeRT = currNodeRT->nextRT;
				cout << "..checking routing table..";
				for (int j = 0; j < identifierValue; j++) {
					if (!currNodeRT) {
						PrintPath(currNode, 0);
						currNode = prevNodeRT->MachPtr;
						break;
					}
					if (currNode->machineID == key) {
						PrintPath(currNode, 0);
						MachineFound = true;
						break;
					}
					else if (j > 0) { // for the values larger than the current key as the list is circular, but numerically smaller
						if (currNode->machineID >= currNodeRT->MachPtr->machineID) {
							PrintPath(currNode, 0);
							currNode = prevNodeRT->MachPtr;
							break;
						}
						else {
							if (prevNodeRT->MachPtr->machineID < key && key <= currNodeRT->MachPtr->machineID) {
								PrintPath(currNode, 0);
								currNode = prevNodeRT->MachPtr;
								break;
							}
							else {
								prevNodeRT = currNodeRT;
								currNodeRT = currNodeRT->nextRT;
							}
						}
					}
					else if (prevNodeRT->MachPtr->machineID < key && key <= currNodeRT->MachPtr->machineID) {
						PrintPath(currNode, 0);
						currNode = prevNodeRT->MachPtr;
						break;
					}
					else {
						prevNodeRT = currNodeRT;
						currNodeRT = currNodeRT->nextRT;
					}

				}
			}
		}
	}
	return currNode;
}

void RingDHT::PrintRouteTable(int MachID) {
	cout << "--------------------------Printing Routing Table of " << MachID << " ---------------------------- - " << endl;
	MachineNode* currNode = headMachine;
	//make Routing Tables if not already made
	if (currNode->headRT == nullptr) {
		CreateRoutingTable();
	}
	currNode = SearchMachine(headMachine, MachID);
	currNode->PrintRoutingTable();
}
//For the insertion and deletion of machine on the fly.
void RingDHT::DeleteRoutingTable(MachineNode* deleteNode) {
	RoutingTableNode* currNode = deleteNode->headRT;
	while (currNode) {
		deleteNode->headRT = currNode->nextRT;
		if (currNode->nextRT) {
			currNode->nextRT->prevRT = nullptr;
		}
		delete currNode;

		currNode = deleteNode->headRT;
	}
}

void RingDHT::insert_Machine(const char* machName) {
	long long int hashed_integer = hashFunction(machName, identifierValue);
	insert_Machine(hashed_integer);
}

void RingDHT::insert_Machine(int idToInsert) {
	cout << "--------------------------Machine Insertion on Fly-----------------------------" << endl;
	long long int range = pow(2, identifierValue);
	if (idToInsert >= range) {
		cout << "The ID cannot be greater than identifier space.\n";
		return;
	}
	//This part inserts the new machine in the circular linked list.
	MachineNode* currNode = headMachine, * newNode = new MachineNode(identifierValue, idToInsert);
	while (currNode->next != headMachine && currNode->next->machineID < idToInsert) {
		currNode = currNode->next;
	}

	if (currNode->next->machineID == idToInsert || idToInsert == headMachine->machineID) {
		cout << "MACHINE with ID " << idToInsert << " already exists.\n";
		return;
	}

	//A function to delete the routing table of each node.
	MachineNode* deleteNode = headMachine;
	while (deleteNode->next->machineID != headMachine->machineID) {
		DeleteRoutingTable(deleteNode);
		deleteNode = deleteNode->next;
	}
	//To delete the routing table of last node.
	DeleteRoutingTable(deleteNode);

	//To deal with insertion at head.
	if (currNode == headMachine) {
		newNode->next = currNode;
		tailMachine->next = headMachine = newNode;
	}
	//To deal with insertion in middle or at end.
	else {
		newNode->next = currNode->next;
		currNode->next = newNode;

		if (currNode == tailMachine) {
			tailMachine = newNode;
		}
	}

	insertedMachines++;		numberOf_Machines++;

	//Function to recreate the routing table of all the machines.
	CreateRoutingTable();

	MachineNode* new_node = newNode, * nextNode = new_node->next;
	int index = newNode->minRange;
	if (new_node == headMachine) {
		while (index <= (pow(2, identifierValue) - 1)) {
			FileNode* nodeToFind = nullptr;
			nextNode->searchForDeletion(index, nodeToFind);

			if (nodeToFind) {
				FileNode* temp = nodeToFind->next;
				while (temp) {
					new_node->insert(index, temp->value, temp->name);
					nextNode->remove(index, temp->name);

					temp = temp->next;
				}
				nextNode->remove(index, nodeToFind->name);
				new_node->insert(index, nodeToFind->value, nodeToFind->name);
			}
			index++;
		}

		index = 0;
		while (index < newNode->machineID) {
			FileNode* nodeToFind = nullptr;
			nextNode->searchForDeletion(index, nodeToFind);

			if (nodeToFind) {
				FileNode* temp = nodeToFind->next;
				while (temp) {
					new_node->insert(index, temp->value, temp->name);
					nextNode->remove(index, temp->name);

					temp = temp->next;
				}

				nextNode->remove(index, nodeToFind->name);
				newNode->insert(index, nodeToFind->value, nodeToFind->name);
			}
			index++;
		}
	}
	else {
		while (index <= new_node->machineID) {
			FileNode* nodeToFind = nullptr;
			nextNode->searchForDeletion(index, nodeToFind);

			if (nodeToFind) {
				FileNode* temp = nodeToFind->next;
				while (temp) {
					new_node->insert(index, temp->value, temp->name);
					nextNode->remove(index, temp->name);

					temp = temp->next;
				}

				nextNode->remove(index, nodeToFind->name);
				new_node->insert(index, nodeToFind->value, nodeToFind->name);
			}
			index++;
		}
	}
}

void RingDHT::delete_Machine(int idToDelete) {
	cout << "--------------------------Machine Deletion on Fly-----------------------------" << endl;
	long long int range = pow(2, identifierValue);
	if (idToDelete >= range) {
		cout << "The ID cannot be greater than identifier space.\n";
		return;
	}

	if (headMachine->next == headMachine) {
		headMachine->machineID = -1;
		headMachine->minRange = -1;
		return;
	}
	//This part inserts the new machine in the circular linked list.
	MachineNode* currNode = headMachine, * nodeToDelete = nullptr;
	while (currNode->next != headMachine && currNode->next->machineID != idToDelete) {
		currNode = currNode->next;
	}

	if (currNode->next->machineID != idToDelete) {
		cout << "MACHINE not found.\n";
		return;
	}

	//A function to delete the routing table of each node.
	MachineNode* deleteNode = headMachine;
	while (deleteNode->next->machineID != headMachine->machineID) {
		DeleteRoutingTable(deleteNode);
		deleteNode = deleteNode->next;
	}
	//To delete the routing table of last node.
	DeleteRoutingTable(deleteNode);

	//Code to delete b-tree nodes from the node to be deleted and insert them in the next node.
	MachineNode* nextNode = currNode->next->next, * nodeDel = currNode->next;
	int index = nodeDel->minRange;

	//Incase, headMachine is being deleted.
	if (nodeDel == headMachine) {
		while (index <= (pow(2, identifierValue) - 1)) {
			FileNode* nodeToFind = nullptr;
			nodeDel->searchForDeletion(index, nodeToFind);

			if (nodeToFind) {
				FileNode* temp = nodeToFind->next;
				while (temp) {
					nextNode->insert(index, temp->value, temp->name);
					nodeDel->remove(index, temp->name);

					temp = temp->next;
				}

				nodeDel->remove(index, nodeToFind->name);
				nextNode->insert(index, nodeToFind->value, nodeToFind->name);
			}
			index++;
		}

		index = 0;
		while (index < nodeDel->machineID) {
			FileNode* nodeToFind = nullptr;
			nodeDel->searchForDeletion(index, nodeToFind);

			if (nodeToFind) {
				FileNode* temp = nodeToFind->next;
				while (temp) {
					nextNode->insert(index, temp->value, temp->name);
					nodeDel->remove(index, temp->name);

					temp = temp->next;
				}

				nodeDel->remove(index, nodeToFind->name);
				nextNode->insert(index, nodeToFind->value, nodeToFind->name);
			}
			index++;
		}
	}
	//Incase any other machine is being deleted.
	else {
		while (index <= nodeDel->machineID) {
			FileNode* nodeToFind = nullptr;
			nodeDel->searchForDeletion(index, nodeToFind);

			if (nodeToFind) {
				FileNode* temp = nodeToFind->next;
				while (temp) {
					nextNode->insert(index, temp->value, temp->name);
					nodeDel->remove(index, temp->name);

					temp = temp->next;
				}

				nodeDel->remove(index, nodeToFind->name);
				nextNode->insert(index, nodeToFind->value, nodeToFind->name);

			}
			index++;
		}
	}

	nodeToDelete = currNode->next;
	//To delete from head.
	if (nodeToDelete == headMachine) {
		headMachine = currNode->next->next;
		tailMachine->next = headMachine;

		delete nodeToDelete;
	}
	//To delete from middle or end.
	else {
		currNode->next = currNode->next->next;

		if (nodeToDelete == tailMachine) {
			tailMachine = nodeToDelete->next;
		}

		delete nodeToDelete;
	}

	insertedMachines--;		numberOf_Machines--;

	//Function to recreate the routing table of all the machines.
	CreateRoutingTable();
}
//For B-tree's searching, insertion and removal

void RingDHT::searchFile(int curr_pos, int file_hash, string name) {
	cout << "--------------------------File Search-----------------------------" << endl;
	MachineNode* machine = SearchMachine(curr_pos, file_hash);
	string path = "";
	if (machine != nullptr) {
		path = machine->search(file_hash, name);
		if (path != "") {
			cout << "Path of File: " << path << endl;
		}
	}
}

void RingDHT::traverseTree(int curr_pos, int machine_hash) {
	cout << "--------------------------MachineNode Traversal-----------------------------" << endl;
	MachineNode* machine = SearchMachine(curr_pos, machine_hash);
	if (machine != nullptr) {
		cout << "The File Nodes existing on " << machine->machineID << " are: ";
		machine->traverse();
	}
	cout << endl;
}

void RingDHT::InsertFile(int curr_pos, int file_hash, string value, string name) {
	MachineNode* machine = SearchMachine(curr_pos, file_hash);
	if (machine) {
		machine->insert(file_hash, value, name);
	}
}

void RingDHT::removeFile(int curr_pos, int file_hash, string naam) {
	cout << "--------------------------File Removal-----------------------------" << endl;
	cout << "Removing file of hash: " << file_hash << endl;
	MachineNode* machine = SearchMachine(curr_pos, file_hash);
	if (machine) {
		machine->remove(file_hash, naam);
	}
}
//To read from file.
string readFile(string nameOfFile, string address, bool& fileRead) {
	string filename = address + "\\" + nameOfFile + ".txt";
	string content = "";

	ifstream MyReadFile;
	MyReadFile.open(filename);

	if (MyReadFile) {
		while (getline(MyReadFile, content)) {
		}
		MyReadFile.close();
		fileRead = true;
	}
	else {
		cout << "File with " << nameOfFile << " does not exist.\n";
		fileRead = false;
	}

	return content;
}

void RingDHT::InsertFile(string fileName, string fileAddress, int posForMachine) {
	cout << "--------------------------File Insertion-----------------------------" << endl;

	bool isFileRead;
	string fileContent = readFile(fileName, fileAddress, isFileRead);

	if (!isFileRead)
		return;
	long long int fileHash = hashFunction(fileContent.c_str(), identifierValue);
	cout << "File Hash: " << fileHash << endl;
	InsertFile(posForMachine, fileHash, fileAddress, fileName);
}
