#include <bits/stdc++.h>
#include "file_manager.h"
#include "errors.h"

using namespace std;

#define im INT32_MIN

int demark = -1, filler = im;

bool recompute_write_head (int &write_page, int &write_offset, int offset_limit) {
    if (write_offset + 2 < offset_limit) {
        write_offset = write_offset + 2;
        return false;
    }
    write_offset = 0;
    write_page += 1;
    return true; 
}

void print_file (FileHandler* fh) {
    PageHandler read = fh->LastPage();
    int last_page_num = read.GetPageNum();

    int num_of_int_each_page = PAGE_CONTENT_SIZE/sizeof(int);  

    for (int i = 0; i <= last_page_num; i++) {
        read = fh->PageAt(i);

        int num; 
        char* read_data = read.GetData();

        cout << "Page Num-" << read.GetPageNum() << " has content:  \t";
        for (int j = 0; j < num_of_int_each_page; j++) {
            memcpy(&num, &read_data[4*j], sizeof(int));
            cout << num << "\t";
        }
        cout << "\n";
        fh->UnpinPage(i);
    }  
    cout << "\n";
}

void linear_search (FileHandler* fh_read, FileHandler* fh_write, int search_for, int &write_page, int &write_offset, int last_page_num, int num_of_int_each_page) {
    PageHandler read = fh_read->FirstPage();
    char* read_data = read.GetData();

    PageHandler write; char* write_data;
    if (write_offset == 0)
        write = fh_write->NewPage();
    else 
        write = fh_write->PageAt(write_page);
    write_data = write.GetData();

    int cur_page = 0;
    bool cont = true;
    while (cur_page <= last_page_num && cont) {
        // cout << "Hello " << cur_page << "\n";
        read = fh_read->PageAt(cur_page);
        char* read_data = read.GetData();

        int num;

        for (int i = 0; i < num_of_int_each_page; i++) {
            memcpy(&num, &read_data[4*i], sizeof(int));

            // cout << num << "\n";
            if (num > search_for) {
                cont = false;
                break;
            }
            else if (num == search_for) {
                // cout << write_page << " " << write_offset << "\n"; 
                memcpy(&write_data[4*write_offset], &cur_page, sizeof(int));
                memcpy(&write_data[4*write_offset + 4], &i, sizeof(int));

                if (recompute_write_head(write_page, write_offset, num_of_int_each_page)) {
                    fh_write->MarkDirty(write_page - 1);
                    fh_write->UnpinPage(write_page - 1);
                    // cout << write_page << "\n";
                    // fh_write->DisposePage(write_page - 1);
                    write = fh_write->NewPage();
                    write_data = write.GetData();
                }
            }
        }
        fh_read->UnpinPage(cur_page);
        cur_page += 1;
    }

    memcpy(&write_data[4*write_offset], &demark, sizeof(int));
    memcpy(&write_data[4*write_offset + 4], &demark, sizeof(int));
    if (recompute_write_head(write_page, write_offset, num_of_int_each_page)) {
        fh_write->MarkDirty(write_page - 1);
        fh_write->UnpinPage(write_page - 1);
    }
    // fh_write->FlushPages();
}

void cleanup_file (FileHandler* fh_write, int num_of_int_each_page, int write_offset) {
    if (write_offset != 0) {
        char* write_data = fh_write->LastPage().GetData();
        for (int i = write_offset; i < num_of_int_each_page; i++) {
            memcpy(&write_data[4*i], &filler, sizeof(int));
        }
    }
}

int main (int argc, char* argv[]) {
    const char* input_file = argv[1]; 
    const char* query_txt = argv[2];
    const char* output_file = argv[3];

    FileManager fm;

    FileHandler fh_read = fm.OpenFile(input_file);

    PageHandler read = fh_read.LastPage();
    int last_page_num = read.GetPageNum();
    int num_of_int_each_page = PAGE_CONTENT_SIZE/sizeof(int);

    // cout << "Input file content :-\n";
    // print_file(&fh_read);

    FileHandler fh_write = fm.CreateFile(output_file);
    int write_page = 0, write_offset = 0;

    fstream query;
    query.open(query_txt);
    string search, to_search;
    while (query >> search) {
        if (search == "SEARCH") {
            query >> to_search;
            int to_search_num = stoi(to_search);
            linear_search(&fh_read, &fh_write, to_search_num, write_page, write_offset, last_page_num, num_of_int_each_page);
        }
    }

    // linear_search(&fh_read, &fh_write, 5, write_page, write_offset, last_page_num, num_of_int_each_page);
    cleanup_file(&fh_write, num_of_int_each_page, write_offset);

    fh_write.FlushPages();
    // print_file(&fh_write);
    fm.CloseFile(fh_write);
    fm.CloseFile(fh_read);

    // FileHandler expected_output = fm.OpenFile("TestCases/TC_search/output_search");
    // print_file(&expected_output);
    // fm.CloseFile(expected_output);
}