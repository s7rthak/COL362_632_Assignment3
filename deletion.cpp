#include <bits/stdc++.h>
#include "file_manager.h"
#include "errors.h"

using namespace std;

#define im INT32_MIN

int demark = -1, filler = im;

bool recompute_write_head (int &write_page, int &write_offset, int offset_limit) {
    if (write_offset + 1 < offset_limit) {
        write_offset = write_offset + 1;
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

bool interchange_data (FileHandler* fh, int a_page, int a_offset, int b_page, int b_offset) {
    PageHandler b = fh->PageAt(b_page);
    char* b_data = b.GetData();

    int b_num;
    memcpy(&b_num, &b_data[4*b_offset], sizeof(int));

    if (b_num == im) return false;

    PageHandler a = fh->PageAt(a_page);
    char* a_data = a.GetData();
    // cout << "Here\n";

    int a_num;
    memcpy(&a_num, &a_data[4*a_offset], sizeof(int));

    memcpy(&a_data[4*a_offset], &b_num, sizeof(int));
    memcpy(&b_data[4*b_offset], &a_num, sizeof(int));

    fh->MarkDirty(a_page);
    fh->UnpinPage(a_page);
    fh->MarkDirty(b_page);
    fh->UnpinPage(b_page);

    return true;
}

void cleanup_pages (FileHandler* fh) {
    while (true) {
        PageHandler last = fh->LastPage();
        char* l_data = last.GetData();

        int num;
        memcpy(&num, &l_data[0], sizeof(int));

        if (num == im) {
            fh->DisposePage(last.GetPageNum());
        } else {
            break;
        }

        fh->UnpinPage(last.GetPageNum());
    }
}

void mark_all_dirty_and_flush (FileHandler* fh) {
    for (int i = 0; i <= fh->LastPage().GetPageNum(); i++) {
        fh->MarkDirty(i);
        fh->UnpinPage(i);
    }
    // fh->UnpinPages();
}

void deletion (FileHandler* fh_write, int search_for, int last_page_num, int num_of_int_each_page) {
    fh_write->UnpinPage(last_page_num);
    int start_page = -1, start_offset = -1, end_page = -1, end_offset = -1;

    int l_range = 0, r_range = last_page_num;
    while (l_range != r_range) {
        // cout << "Hello " << search_for << "\n" ;
        int mid = (l_range + r_range + 1) / 2;
        PageHandler read = fh_write->PageAt(mid);
        char* read_data = read.GetData();

        int num;
        memcpy(&num, &read_data[0], sizeof(int));
        if (num < search_for) {
            l_range = mid;
        } else {
            r_range = mid - 1;
        }
        fh_write->UnpinPage(mid);
    }

    bool cont = true, first = true;
    int cur_page = l_range;
    while (cur_page <= last_page_num && cont) {
        PageHandler write = fh_write->PageAt(cur_page);
        char* write_data = write.GetData();

        int num;

        for (int i = 0; i < num_of_int_each_page; i++) {
            memcpy(&num, &write_data[4*i], sizeof(int));

            if (num > search_for) {
                cont = false;
                break;
            } else if (num == search_for) {
                if (first) {
                    start_page = cur_page;
                    start_offset = i;
                    first = false;
                }
                memcpy(&write_data[4*i], &filler, sizeof(int));
                end_page = cur_page;
                end_offset = i;
            }
        }
        fh_write->MarkDirty(cur_page);
        fh_write->UnpinPage(cur_page);
        cur_page += 1;
    }

    if (start_page == -1 || start_offset == -1 || end_page == -1 || end_offset == -1) return;
    recompute_write_head(end_page, end_offset, num_of_int_each_page);

    while (end_page <= last_page_num) {
        // cout << start_page << " " << start_offset << " " << end_page << " " << end_offset << "\n";
        if (interchange_data(fh_write, start_page, start_offset, end_page, end_offset)) {
            // cout << "Reached here\n";
            recompute_write_head(start_page, start_offset, num_of_int_each_page);
            recompute_write_head(end_page, end_offset, num_of_int_each_page);
        } else {
            break;
        }
    }
    
    cleanup_pages(fh_write);
    // mark_all_dirty_and_flush(fh_write);
    // print_file(fh_write);
}

void cleanup_file (FileHandler* fh_write, int num_of_int_each_page, int write_offset) {
    if (write_offset != 0) {
        char* write_data = fh_write->LastPage().GetData();
        for (int i = write_offset; i < num_of_int_each_page; i++) {
            memcpy(&write_data[4*i], &filler, sizeof(int));
        }
        // fh_write->UnpinPage(fh_write->LastPage().GetPageNum());
    }
}

int main (int argc, char* argv[]) {
    const char* input_file = argv[1]; 
    const char* query_txt = argv[2];

    FileManager fm;

    FileHandler fh_read = fm.OpenFile(input_file);
    // FileHandler fh_write = FileHandler(fh_read);
    // print_file(&fh_read);

    int num_of_int_each_page = PAGE_CONTENT_SIZE/sizeof(int);
    // int last_page_num = fh_write.LastPage().GetPageNum();

    fstream query;
    query.open(query_txt);
    string del, to_del;
    while (query >> del) {
        if (del == "DELETE") {
            query >> to_del;
            int to_del_num = stoi(to_del);
            deletion(&fh_read, to_del_num, fh_read.LastPage().GetPageNum(), num_of_int_each_page);
        }
    }
    
    // print_file(&fh_read);
    fm.CloseFile(fh_read);

    // FileHandler expected_output = fm.OpenFile("TestCases/TC_delete/output_delete");
    // print_file(&expected_output);
    // fm.CloseFile(expected_output);
}