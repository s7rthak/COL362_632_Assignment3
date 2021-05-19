#include <bits/stdc++.h>
#include "file_manager.h"
#include "errors.h"

using namespace std;

#define im INT32_MIN

int demark = -1, filler = im;

void print_file (FileHandler* fh) {
    PageHandler read = fh->LastPage();
    int last_page_num = read.GetPageNum();

    // vector<int> occ_counter (10, 0);

    int num_of_int_each_page = PAGE_CONTENT_SIZE/sizeof(int);  

    for (int i = 0; i <= last_page_num; i++) {
        read = fh->PageAt(i);

        int num; 
        char* read_data = read.GetData();

        cout << "Page Num-" << read.GetPageNum() << " has content:  \t";
        for (int j = 0; j < num_of_int_each_page; j++) {
            memcpy(&num, &read_data[4*j], sizeof(int));
            cout << num << "\t";
            // if (num != im)
            // occ_counter[num] += 1;
        }
        cout << "\n";
        fh->UnpinPage(i);
    }  
    // cout << "Occurences:\n";
    // for (int i = 1; i < occ_counter.size(); i++) {
    //     cout << i << " - " << occ_counter[i] << "\n";
    // }
    cout << "\n";
}

bool recompute_write_head (int &write_page, int &write_offset, int offset_limit) {
    if (write_offset + 1 < offset_limit) {
        write_offset = write_offset + 1;
        return false;
    }
    write_offset = 0;
    write_page += 1;
    return true; 
}

void take_in_pages (FileHandler* fh, vector<PageHandler> &take, int start, int max_take, int last_page) {
    for (int i = start; i < start + max_take && i <= last_page; i++) {
        PageHandler ph = fh->PageAt(i);
        take.push_back(ph);
    }
}
 
void flush_taken_pages (FileHandler* fh, int start, int max_take, int last_page) {
    for (int i = start; i < start + max_take && i <= last_page; i++) {
        fh->UnpinPage(i);
    }
}

void cleanup_file (FileHandler* fh_write, int num_of_int_each_page, int write_offset) {
    if (write_offset != 0) {
        char* write_data = fh_write->LastPage().GetData();
        for (int i = write_offset; i < num_of_int_each_page; i++) {
            memcpy(&write_data[4*i], &filler, sizeof(int));
        }
        fh_write->UnpinPage(fh_write->LastPage().GetPageNum());
    } else {
        fh_write->DisposePage(fh_write->LastPage().GetPageNum());
    } 
}

// 0 - output buffer
// 1 - in1 buffer
// rest - in2 buffer
void join1 (FileHandler* in1, FileHandler* in2, FileHandler* out, int n, int num_int_each_page) {
    int in1_num_page = in1->LastPage().GetPageNum();
    in1->UnpinPage(in1_num_page);
    int in2_num_page = in2->LastPage().GetPageNum();
    in2->UnpinPage(in2_num_page);

    int in1_page = 0, in2_page = 0, out_page = 0, out_offset = 0;
    
    PageHandler out_buf = out->NewPage();

    while (in1_page <= in1_num_page) {
        PageHandler in1_buf = in1->PageAt(in1_page);
        char* in1_data = in1_buf.GetData(); 

        in2_page = 0;
        while (in2_page <= in2_num_page) {
            vector<PageHandler> in2_buf_arr;
            take_in_pages(in2, in2_buf_arr, in2_page, n - 2, in2_num_page);

            int num1, num2;

            for (int i = 0; i < num_int_each_page; i++) {
                memcpy(&num1, &in1_data[4*i], sizeof(int));

                for (int j = 0; j < in2_buf_arr.size(); j++) {
                    char* in2_data = in2_buf_arr[j].GetData();

                    for (int k = 0; k < num_int_each_page; k++) {
                        memcpy(&num2, &in2_data[4*k], sizeof(int));

                        if (num1 == num2 && num1 != im) {
                            char* out_data = out_buf.GetData();
                            memcpy(&out_data[4*out_offset], &num1, sizeof(int));
                            out->MarkDirty(out_page);

                            if (recompute_write_head(out_page, out_offset, num_int_each_page)) {
                                // cout << "Reached Here " << out_page << "\n";
                                out->UnpinPage(out_page - 1);
                                out_buf = out->NewPage();
                            }
                        }
                    }
                }
            }

            flush_taken_pages(in2, in2_page, n - 2, in2_num_page);
            in2_page += (n - 2);
        }

        in1->UnpinPage(in1_page);
        in1_page += 1;
    }

    cleanup_file (out, num_int_each_page, out_offset);
}

int main (int argc, char* argv[]) {
    const char* input_file1 = argv[1]; 
    const char* input_file2 = argv[2];
    const char* output_file = argv[3];

    FileManager fm;

    FileHandler fh_input1 = fm.OpenFile(input_file1);
    // print_file(&fh_input1);

    FileHandler fh_input2 = fm.OpenFile(input_file2);
    // print_file(&fh_input2);

    int num_of_int_each_page = PAGE_CONTENT_SIZE/sizeof(int);

    FileHandler output = fm.CreateFile(output_file);
    join1(&fh_input1, &fh_input2, &output, BUFFER_SIZE, num_of_int_each_page);
    output.FlushPages();
    // print_file(&output);

    fm.CloseFile(fh_input1);
    fm.CloseFile(fh_input2);
    fm.CloseFile(output);

    // FileHandler expected_output = fm.OpenFile("TestCases/TC_join2/output_join2");
    // print_file(&expected_output);
    // fm.CloseFile(expected_output);
}