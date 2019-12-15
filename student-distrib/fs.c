#include "fs.h"

// local function prototypes
uint32_t min(uint32_t a, uint32_t b, uint32_t c);


/*
 * Function takes no arguments. Should only be called on initailization of system.
 * Called in kernel.c
 * INPUTS: None
 * OUTPUTS: None
 * SIDEEFFECTS: edits global variable fs to take data from module 0 loaded in kernel.c
 * If call again, will overwrite file data if already opened
*/
void fs_init(){
    fs.num_dir_entries = *fs_ptr;   
    fs.num_inodes = *(fs_ptr + B4);
    fs.num_data_blocks = *(fs_ptr + B8);
    fs.dir_entries_ptr = fs_ptr + B64;
}

/*
 * Function reads from currently open file in fs struct. Uses local function read_data 
 * and must make sure not to overfill buffer
 * INPUTS: pointer to buffer to copy data, and number of bytes to copy
 * OUTPUTS: number of bytes returned. 
 * SIDEEFFECTS: Will overwrite buffer data
*/
int32_t file_read(int32_t fd, void* buf, int32_t num_of_bytes){
    uint32_t cursor_movement;
    // sanity checks
    if(num_of_bytes < 0){
        //printf("\nERROR: Cannot copy negative number of bytes.\nnum_of_bytes = %d", num_of_bytes);
        return 0;
    }
    if(cur_pid < 0 || cur_pid >= MAX_PCBS){     // cur_pid is not junk value
        return -1;
    }
    else if(pcb_ptr_array[cur_pid] == NULL){    // process has been initialized
        return -1;
    }
    else if(pcb_ptr_array[cur_pid]->file_desc_array[fd].flags == 0){ // a file is open in fd
        return -1;
    }

    // this will later be updated to use a File Descriptor. 
    cursor_movement = read_data(pcb_ptr_array[cur_pid]->file_desc_array[fd].inode_index, 
                                pcb_ptr_array[cur_pid]->file_desc_array[fd].file_position, 
                                (uint8_t*)buf, (uint32_t)num_of_bytes);
    pcb_ptr_array[cur_pid]->file_desc_array[fd].file_position += cursor_movement;
    return cursor_movement;
}

/*
 * Function takes a buffer and returns the name of file in directory entry refrenced by the
 * dir_cursor field of fs struct. the dir_cursor indicates directory index, not a byte wise cursor
 * like file_cursor.
 * INPUT: buffer pointer, number of bytes to copy
 * OUTPUT: number of bytes read
 * SIDEEFFECT: copies over name of file into the buffer provided
*/
int32_t dir_read(int32_t fd, void* buf, int32_t num_of_bytes){
    uint32_t num_bytes_to_copy;
    int32_t dir_cursor;
    // sanity checks
    if(cur_pid < 0 || cur_pid >= MAX_PCBS){     // cur_pid is not junk value
        return 0;
    }
    else if(pcb_ptr_array[cur_pid] == NULL){    // process has been initialized
        return 0;
    }
    else if(pcb_ptr_array[cur_pid]->file_desc_array[fd].flags == 0){ // a file is open in fd
        return 0;
    }
    if(num_of_bytes < 0){
        //printf("\nERROR: Cannot copy negative number of bytes.\nnum_of_bytes = %d", num_of_bytes);
        return 0;
    }

    else if(pcb_ptr_array[cur_pid]->file_desc_array[fd].file_position >= fs.num_dir_entries){
        //printf("\nReached End of Directory.");
        return 0;
    }
    dir_cursor = pcb_ptr_array[cur_pid]->file_desc_array[fd].file_position;
    
    num_bytes_to_copy = min((uint32_t)num_of_bytes, strlen((int8_t*)(fs.dir_entries_ptr + dir_cursor*B64)), B32*4);
    memcpy(buf, (fs.dir_entries_ptr + dir_cursor*B64), num_bytes_to_copy);
    pcb_ptr_array[cur_pid]->file_desc_array[fd].file_position++;
    return num_bytes_to_copy;
}


/*
 * Function takes pointer to dentry field and file name, and attempts to copy the directory entry
 * data into the pointer provided by caller. Will return 0 for success or -1 for failure
 * INPUT: string name, dentry struct pointer
 * OUTPUT: 0 for success, -1 for failure 
 * SIDEEFFECTS: writes over data in dentry 
*/
int32_t read_dentry_by_name(const uint8_t* file_name, dentry_t* dentry){
    uint32_t index; 
    uint32_t name_len = strlen((int8_t*)file_name);
    
    if(dentry == NULL){
        //printf("\nError: Provided null pointer.");
        return -1;
    }

    // check that file name is of expected size, note that 32nd char should be null
    if(name_len > MAX_FILE_NAME_LEN + 1){
        //printf("\nError: File name is too long.");
        return -1;
    }

    for(index = 0; index <= MAX_FILE_NUM; index++){
        // strncmp returns 0 if strings match 
        if(!strncmp((int8_t*)file_name, (int8_t*)(fs.dir_entries_ptr + index*B64), MAX_FILE_NAME_LEN+1)){
            // check that file_name is not "." ie. directory index = 0
            if(!index){
                //printf("\nError: A directory is not a valid file.");
                //return -1;
            }
            //printf("\nSuccess: Found dentry for %s at directory index %d.",file_name, index);
            break;
        }
        // if not broken and last loop, file not found
        else if(index == MAX_FILE_NUM){
            //printf("\nError: File of name %s was not found.", file_name);
            return -1;
        }
    }
    // at this point, index should be the directory index of the file specified by file_name
    // copy over the name, type, and inode index
    strncpy(dentry->file_name, (int8_t*)(fs.dir_entries_ptr + index*B64), name_len);
    dentry->file_type = *(fs.dir_entries_ptr + index*B64 + B32);
    dentry->inode_index = *(fs.dir_entries_ptr + index*B64 + B32+ B4);
    dentry->file_size = *(fs_ptr + (dentry->inode_index + 1) * KB4);
    return 0;
}

/*
 * Function takes pointer to dentry field and directory index, and attempts to copy the directory entry
 * data into the pointer provided by caller. Will return 0 for success or -1 for failure
 * INPUT: uint32 directory index, dentry struct pointer
 * OUTPUT: 0 for success, -1 for failure 
 * SIDEEFFECTS: writes over data in dentry 
*/
int32_t read_dentry_by_index(uint32_t dir_index, dentry_t* dentry){

    // check that entry data is valid
    if(!dir_index || dir_index > MAX_FILE_NUM){
        //printf("\nERROR: Provided dir_index is either 0 or too large. dir_index = %d.", dir_index);
        return -1;
    }
    // check for null pointer
    if(dentry == NULL){
        //printf("\nERROR: Provided NULL pointer.");
        return -1;
    }

    strcpy(dentry->file_name, (int8_t*)(fs.dir_entries_ptr + dir_index*B64));
    dentry->file_type = *(fs.dir_entries_ptr + dir_index*B64 + B32);
    dentry->inode_index = *(fs.dir_entries_ptr + dir_index*B64 + B32 + B4);
    dentry->file_size = *(fs_ptr + (dentry->inode_index + 1) * KB4);
    return 0;
}

/*
 * Function takes index of an inode, a number of bytes data offset, a pointer to a buffer, and number of bytes 
 * to copy len, and copies data from the appropriate data blocks to the buffer with a max number of len. 
 * INPUT: inode index, offset, buf pointer, and length
 * OUTPUT: -1 for failure, number of bytes copied on success, 0 indicates end of file
 * SIDEEFFECTS: writes over previous data in buf
*/
int32_t read_data(uint32_t inode_index, uint32_t offset, uint8_t* buf, uint32_t len){
    uint32_t file_size_in_bytes;
    uint32_t data_block_offset;
    uint32_t data_byte_offset;
    uint32_t bytes_left_in_file;
    uint32_t bytes_left_to_copy;
    uint32_t bytes_left_in_cur_block;
    uint32_t num_bytes_copied;
    uint32_t* data_block_index_ptr;
    uint8_t* data_ptr;

    // check that inode_index is valid
    if(inode_index >= fs.num_inodes){
        //printf("Error: inode index %d is out of range.", inode_index);
        return -1; 
    }

    // get length of file in bytes
    file_size_in_bytes = *(fs_ptr + (inode_index + 1) * KB4);
    
    if(offset >= file_size_in_bytes){
        //printf("\nOffset reaches end of file.");
        return 0; // for 0 bytes moved
    }

    // mod offset with 4096 since each data block is 4kb and constant is for int32 pointer
    // and then get remainder of bytes offset within data block  
    data_block_offset = offset / (KB4*4);
    data_byte_offset = offset - (data_block_offset*KB4*4);  

    // point to first data block # -> fs_start + skip_boot_block + inode_index + skip_length_entry + block_offset
    data_block_index_ptr = fs_ptr + KB4 + KB4*inode_index + B4 + data_block_offset;

    // check that data block entry is valid
    if(*data_block_index_ptr >= fs.num_data_blocks){
        //printf("ERROR: Data block # in inode is bad.\nData Block index = %d\nNumber of Blocks = %d",
        //*data_block_index_ptr,fs.num_data_blocks);
        //    *data_block_index_ptr,fs.num_data_blocks);
        return -1;
    }

    bytes_left_to_copy = len;
    bytes_left_in_file = file_size_in_bytes - offset;
    bytes_left_in_cur_block = KB4*4 - data_byte_offset;

    /*
        while loop copies over the min of bytes left in block,bytes left in file, bytes left to copy,
        because each may be a constraint. Besides first and last data block, each memcpy should copy
        an entire data block. When a block has been read to completion, pointer in inode is updated
        to point to the next data block index which is used to point to the data block in memory
    */
    while(bytes_left_in_file > 0 && bytes_left_to_copy > 0){
        // if we finish current block, update pointer in inode to next data block index, and reset bytes in block
        if(bytes_left_in_cur_block == 0){
            data_block_index_ptr++;
            bytes_left_in_cur_block = KB4*4;
        }
        // check that data block entry is valid
        if(*data_block_index_ptr >= fs.num_data_blocks){
            //printf("ERROR: Data block # in inode is bad.\nData Block index = %d\nNumber of Blocks = %d",
            //*data_block_index_ptr,fs.num_data_blocks);
            //    *data_block_index_ptr,fs.num_data_blocks);
            return -1;
        }

        // point to address of current byte to copy in current data block.
        // multiply constants by 4 since data_ptr is uint8_t pointer
        data_ptr = (uint8_t*)(fs_ptr + KB4*(1 + fs.num_inodes + (*data_block_index_ptr))) + data_byte_offset;
        
        // get number of bytes to use in memcpy call
        num_bytes_copied = min(bytes_left_to_copy, bytes_left_in_cur_block, bytes_left_in_file);
        
        // copy data
        memcpy(buf + len - bytes_left_to_copy, data_ptr, num_bytes_copied);

        // update parameters
        bytes_left_to_copy -= num_bytes_copied;
        bytes_left_in_cur_block -= num_bytes_copied;
        bytes_left_in_file -= num_bytes_copied;
    }

    return len - bytes_left_to_copy;
}

/*
    Simple min function returns min value of three unsigned ints.
    Used to determine how many bytes to copy from a file
*/
uint32_t min(uint32_t a, uint32_t b, uint32_t c){
    if(a < b){
        if(a < c){
            return a;
        }
        else{
            return c;
        }
    }
    else{
        if(b < c){
            return b;
        }
        else{
            return c;
        }
    }
}
