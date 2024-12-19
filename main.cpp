#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
using namespace std;

#include "Project.h"


int main() {
	int identifierSpace, num_mach, option1, file_hash, curr_pos;
	string insertMachName, fileAdd;
	//char filename[20];
	int insertMachID;
	char manual_ass;
	string filename;

	// initialize
	cout << "Please enter the following to make IPFS.\n1)Identifier Space(bits): ";
	cin >> identifierSpace;
	cout << "2)Number of Machines: ";
	cin >> num_mach;

	RingDHT R_DHT(identifierSpace, num_mach);

	// build linked list
	cout << "Would You Like to manually assign the machine ids? Y/N\n->";
	cin >> manual_ass;
	if (manual_ass == 'N' || manual_ass == 'n') {
		R_DHT.insertID_automatically();
	}
	else if (manual_ass == 'Y' || manual_ass == 'y') {
		cout << "Insert Machine Name -> 1\nInsert Machine ID -> 2\n->";
		cin >> option1;
		if (option1 == 1) {
			for (int i = 0; i < num_mach; i++) {
				cout << "name: ";
				cin >> insertMachName;
				R_DHT.insertWith_MachineName(insertMachName.c_str());
			}
		}
		else {
			cout << "Please Enter Machines in Ascending order of ID\n";
			for (int i = 0; i < num_mach; i++) {
				cout << "index " << i << " ";
				cin >> insertMachID;
				R_DHT.insertWith_ID(insertMachID, i + 1);
			}

		}
	}

	// enter current machine id
	cout << "Please enter current position: ";
	cin >> curr_pos;

	cout << "What would you like to do?\nPrint Routing Table -> 1\nInsert File -> 2\nDelete File -> 3\nPrint B-Tree -> 4\nInsert Machine -> 5\nDelete Machine -> 6\n->";
	cin >> option1;
	while (option1 > 0) {
		if (option1 == 1) {
			// print routing table of choice
			cout << "Which Routing Table would you like to print?\n->";
			cin >> insertMachID;
			R_DHT.PrintRouteTable(insertMachID);
		}
		else if (option1 == 2) {
			// insert file
			cout << "Inorder to Insert File, enter the following.\n1)Filename: ";
			cin >> filename;
			//cin.getline(filename, 20);
			//getline(cin, filename);
			cout << "2)File Path: ";
			cin >> fileAdd;
			R_DHT.InsertFile(filename, fileAdd, curr_pos);
		}
		else if (option1 == 3) {
			// delete file
			cout << "Inorder to Delete File, enter the following.\n1)File hash: ";
			cin >> file_hash;
			cout << "2)File name: ";
			cin >> filename;
			//getline(cin, 20);
			//getline(cin, filename);
			R_DHT.removeFile(curr_pos, file_hash, filename);
		}
		else if (option1 == 4) {
			// print b - tree
			cout << "Which B-Tree would you like to print? Enter machine id!\n->";
			cin >> insertMachID;
			R_DHT.traverseTree(curr_pos, insertMachID);
		}
		else if (option1 == 5) {
			// insert machine
			cout << "Insert Machine Name -> 1\nInsert Machine ID -> 2\n->";
			cin >> option1;
			if (option1 == 1) {
				// insert machine by name
				cout << "Inorder to Insert Machine, enter the following.\n1)Machine name: ";
				cin >> insertMachName;
				R_DHT.insert_Machine(insertMachName.c_str());
			}
			else if (option1 == 2) {
				// insert machine by id
				cout << "Inorder to Insert Machine, enter the following.\n1)Machine hash: ";
				cin >> insertMachID;
				R_DHT.insert_Machine(insertMachID);
			}
		}
		else if (option1 == 6) {
			// delete machine
			cout << "Inorder to Remove Machine, enter the following.\n1)Machine hash: ";
			cin >> insertMachID;
			R_DHT.delete_Machine(insertMachID);
		}
		cout << "\nWhat would you like to do?\nPrint Routing Table -> 1\nInsert File -> 2\nDelete File -> 3\nPrint B-Tree -> 4\nInsert Machine -> 5\nDelete Machine -> 6\nEnter -1 to Stop!\n->";
		cin >> option1;
	}


	//int identifierBits = 5, numMachines = 1;
	//RingDHT R_DHT(identifierBits, numMachines);

	/////////////////////////////////////////////////////// LIST # 1
	//R_DHT.insertWith_ID(2, 1);
	//R_DHT.insertWith_ID(4, 2);
	//R_DHT.insertWith_ID(9, 3);
	//R_DHT.insertWith_ID(11, 4);
	//R_DHT.insertWith_ID(14, 5);
	//R_DHT.insertWith_ID(18, 6);
	//R_DHT.insertWith_ID(20, 7);
	//R_DHT.insertWith_ID(21, 8);
	//R_DHT.insertWith_ID(28, 9);

	//R_DHT.InsertFile("File1", "Files", 1);
	//R_DHT.InsertFile("File2", "Files", 1);
	//R_DHT.InsertFile("File3", "Files", 1);
	//R_DHT.InsertFile("File4", "Files", 1);
	//R_DHT.InsertFile("File5", "Files", 1);
	//R_DHT.InsertFile("File6", "Files", 1);
	//R_DHT.InsertFile("File7", "Files", 1);
	//R_DHT.InsertFile("File8", "Files", 1);
	//R_DHT.InsertFile("File9", "Files", 1);
	//R_DHT.InsertFile("File10", "Files", 1);
	//R_DHT.InsertFile("File11", "Files", 1);
	//R_DHT.InsertFile("File12", "Files", 1);
	//R_DHT.InsertFile("File13", "Files", 1);
	//R_DHT.InsertFile("File14", "Files", 1);
	//R_DHT.InsertFile("File15", "Files", 1);
	//R_DHT.InsertFile("File16", "Files", 1);
	//R_DHT.InsertFile("File17", "Files", 1);
	//R_DHT.InsertFile("File18", "Files", 1);
	//R_DHT.InsertFile("File19", "Files", 1);
	//R_DHT.InsertFile("File20", "Files", 1);
	//R_DHT.InsertFile("File21", "Files", 1);
	//R_DHT.InsertFile("File22", "Files", 1);
	//R_DHT.InsertFile("File23", "Files", 1);
	//R_DHT.InsertFile("File24", "Files", 1);
	//R_DHT.InsertFile("File25", "Files", 1);
	//R_DHT.InsertFile("File26", "Files", 1);
	//R_DHT.InsertFile("File27", "Files", 1);
	//R_DHT.InsertFile("File28", "Files", 1);
	//R_DHT.InsertFile("File29", "Files", 1);
	//R_DHT.InsertFile("File30", "Files", 1);

	//R_DHT.traverseTree(1, 3);
	//R_DHT.traverseTree(2, 16);
	//R_DHT.traverseTree(2, 24);
	//R_DHT.traverseTree(2, 36);
	//R_DHT.traverseTree(2, 41);
	//R_DHT.traverseTree(2, 72);
	//R_DHT.traverseTree(2, 89);
	//R_DHT.traverseTree(2, 100);

	//R_DHT.insert_Machine(1);
	//R_DHT.traverseTree(1, 1);
	//R_DHT.insert_Machine(5);
	//R_DHT.traverseTree(1, 5);
	//R_DHT.insert_Machine(8);
	//R_DHT.traverseTree(1, 8);
	//R_DHT.insert_Machine(14);
	//R_DHT.traverseTree(1, 14);
	//R_DHT.insert_Machine(25);
	//R_DHT.traverseTree(1, 25);
	//R_DHT.traverseTree(1, 1);
	//R_DHT.insert_Machine(70);
	//R_DHT.traverseTree(1, 70);
	//R_DHT.insert_Machine(92);
	//R_DHT.traverseTree(1, 92);

	//R_DHT.delete_Machine(16);
	//R_DHT.traverseTree(1, 24);
	//R_DHT.delete_Machine(72);
	//R_DHT.traverseTree(1, 89);
	//R_DHT.delete_Machine(110);
	//R_DHT.delete_Machine(1);

	//R_DHT.removeFile(2, 21, "File8");
	//R_DHT.removeFile(2, 26, "File2");
	//R_DHT.removeFile(2, 30, "File3");
	//R_DHT.removeFile(2, 12, "File4");
	//R_DHT.removeFile(2, 4, "File5");
	//R_DHT.removeFile(2, 8, "File6");
	//R_DHT.removeFile(2, 14, "File7");
	//R_DHT.removeFile(2, 21, "File1");
	//R_DHT.removeFile(2, 18, "File9");
	//R_DHT.removeFile(2, 9, "File10");

	//int identifierBits = 20, numMachines = 20;
	//RingDHT R_DHT(identifierBits, numMachines);

	//R_DHT.insertWith_ID(1000, 1);
	//R_DHT.insertWith_ID(7000, 2);
	//R_DHT.insertWith_ID(12000, 3);
	//R_DHT.insertWith_ID(19000, 4);
	//R_DHT.insertWith_ID(263000, 5);
	//R_DHT.insertWith_ID(284700, 6);
	//R_DHT.insertWith_ID(347400, 7);
	//R_DHT.insertWith_ID(393200, 8);
	//R_DHT.insertWith_ID(407800, 9);
	//R_DHT.insertWith_ID(470500, 10);
	//R_DHT.insertWith_ID(510200, 11);
	//R_DHT.insertWith_ID(589900, 12);
	//R_DHT.insertWith_ID(630200, 13);
	//R_DHT.insertWith_ID(790200, 14);
	//R_DHT.insertWith_ID(823600, 15);
	//R_DHT.insertWith_ID(855800, 16);
	//R_DHT.insertWith_ID(920300, 17);
	//R_DHT.insertWith_ID(934900, 18);
	//R_DHT.insertWith_ID(1002300, 19);
	//R_DHT.insertWith_ID(1020600, 20);

	//R_DHT.InsertFile("File1", "Files", 1);
	//R_DHT.InsertFile("File2", "Files", 1);
	//R_DHT.InsertFile("File3", "Files", 1);
	//R_DHT.InsertFile("File4", "Files", 1);
	//R_DHT.InsertFile("File5", "Files", 1);
	//R_DHT.InsertFile("File6", "Files", 1);
	//R_DHT.InsertFile("File7", "Files", 1);
	//R_DHT.InsertFile("File8", "Files", 1);
	//R_DHT.InsertFile("File9", "Files", 1);
	//R_DHT.InsertFile("File10", "Files", 1);
	//R_DHT.InsertFile("File11", "Files", 1);
	//R_DHT.InsertFile("File12", "Files", 1);
	//R_DHT.InsertFile("File13", "Files", 1);
	//R_DHT.InsertFile("File14", "Files", 1);
	//R_DHT.InsertFile("File15", "Files", 1);
	//R_DHT.InsertFile("File16", "Files", 1);
	//R_DHT.InsertFile("File17", "Files", 1);
	//R_DHT.InsertFile("File18", "Files", 1);
	//R_DHT.InsertFile("File19", "Files", 1);
	//R_DHT.InsertFile("File20", "Files", 1);
	//R_DHT.InsertFile("File21", "Files", 1);
	//R_DHT.InsertFile("File22", "Files", 1);
	//R_DHT.InsertFile("File23", "Files", 1);
	//R_DHT.InsertFile("File24", "Files", 1);
	//R_DHT.InsertFile("File25", "Files", 1);
	//R_DHT.InsertFile("File26", "Files", 1);
	//R_DHT.InsertFile("File27", "Files", 1);
	//R_DHT.InsertFile("File28", "Files", 1);
	//R_DHT.InsertFile("File29", "Files", 1);
	//R_DHT.InsertFile("File30", "Files", 1);

	//cout << "------------------------------------------------------------------\n";
	//R_DHT.traverseTree(1,1000);
	//cout << "------------------------------------------------------------------\n";
	//R_DHT.traverseTree(1,7000);
	//cout << "------------------------------------------------------------------\n";
	//R_DHT.traverseTree(1,12000);
	//cout << "------------------------------------------------------------------\n";
	//R_DHT.traverseTree(1,19000);
	//cout << "------------------------------------------------------------------\n";
	//R_DHT.traverseTree(1,263000);
	//cout << "------------------------------------------------------------------\n";
	//R_DHT.traverseTree(1,284700);
	//cout << "------------------------------------------------------------------\n";
	//R_DHT.traverseTree(1,347400);
	//cout << "------------------------------------------------------------------\n";
	//R_DHT.traverseTree(1,393200);
	//cout << "------------------------------------------------------------------\n";
	//R_DHT.traverseTree(1,407800);
	//cout << "------------------------------------------------------------------\n";
	//R_DHT.traverseTree(1,470500);
	//cout << "------------------------------------------------------------------\n";
	//R_DHT.traverseTree(1,510200);
	//cout << "------------------------------------------------------------------\n";
	//R_DHT.traverseTree(1,589900);
	//cout << "------------------------------------------------------------------\n";
	//R_DHT.traverseTree(1,630200);
	//cout << "------------------------------------------------------------------\n";
	//R_DHT.traverseTree(1,790200);
	//cout << "------------------------------------------------------------------\n";
	//R_DHT.traverseTree(1,823600);
	//cout << "------------------------------------------------------------------\n";
	//R_DHT.traverseTree(1,855800);
	//cout << "------------------------------------------------------------------\n";
	//R_DHT.traverseTree(1,920300);
	//cout << "------------------------------------------------------------------\n";
	//R_DHT.traverseTree(1,934900);
	//cout << "------------------------------------------------------------------\n";
	//R_DHT.traverseTree(1,1002300);
	//cout << "------------------------------------------------------------------\n";
	//R_DHT.traverseTree(1,1020600);
	//cout << "------------------------------------------------------------------\n";
	return 0;

}


