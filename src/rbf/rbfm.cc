#include "rbfm.h"
#include "pfm.h"
#include <stdio.h>
#include <string.h>
#include<cmath>
#include<iostream>
using namespace std;
RecordBasedFileManager* RecordBasedFileManager::_rbf_manager = 0;
PagedFileManager* RecordBasedFileManager::_pf_manager = PagedFileManager::instance();


RecordBasedFileManager* RecordBasedFileManager::instance()
{
    if(!_rbf_manager)
        _rbf_manager = new RecordBasedFileManager();

    return _rbf_manager;
}

RecordBasedFileManager::RecordBasedFileManager()
{
}

RecordBasedFileManager::~RecordBasedFileManager()
{
}

RC RecordBasedFileManager::createFile(const string &fileName) {
//	cout << "CreateFile" << endl;
	if(_pf_manager->createFile(fileName)!=0){
		perror("Create file failure!");
		return -1;
	}
	pagedirectory_num = 0;
    return 0;
}

RC RecordBasedFileManager::destroyFile(const string &fileName) {
//	cout << "DestroyFile" << endl;
	_pf_manager->destroyFile(fileName);
    return 0;
}

RC RecordBasedFileManager::openFile(const string &fileName, FileHandle &fileHandle) {
//	cout << "OpenFile" << endl;
	if(_pf_manager->openFile(fileName,fileHandle)!=0){
		perror("Open file failure!");
		return -1;
	}
	if(pagedirectory_num==0){
		PageDirectory pd;
		pd.encodePageDirectory();
		fileHandle.appendPage(pd.buffer);
		pagedirectory_num = 1;
		current_pagedirectory = 0;
	}
    return 0;
}

RC RecordBasedFileManager::closeFile(FileHandle &fileHandle) {
//	cout << "CloseFile" << endl;
	_pf_manager->closeFile(fileHandle);
    return 0;
}

RC RecordBasedFileManager::insertRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const void *data, RID &rid) {

	int page_num = fileHandle.getNumberOfPages();
//	cout <<"0:"<< pagedirectory_num << ":"<<current_pagedirectory << ":" << page_num << endl;

	//	cout << "InsertRecord" << endl;
//	cout<<"0";
	Record record(recordDescriptor,data);
//	cout<<"1";
	char* pd_tmp = (char*)malloc(PAGE_SIZE);
	fileHandle.readPage(current_pagedirectory,pd_tmp);
//	cout<<"2";
	PageDirectory pd(pd_tmp);
//	cout << (PAGE_SIZE-2*sizeof(int))/sizeof(PageDirectorySlot) << endl;
	if(pd.num_page == (PAGE_SIZE-2*sizeof(int))/sizeof(PageDirectorySlot)){
		pd.next_page = page_num;
//		cout << "&" << current_pagedirectory <<"&"<< pd.next_page<<endl;

//		cout << "$#" << pd.next_page << endl;
		pd.encodePageDirectory();

		fileHandle.writePage(current_pagedirectory,pd.buffer);
//		cout << pd.next_page << endl;


//		free(&pd);

		PageDirectory pd1;
		pd = pd1;
//		cout << pd.num_page << endl;
		pd.num_page = 0;
		pd.encodePageDirectory();
//		cout<< "ri" <<pd.pds.size()<< endl;

		fileHandle.appendPage(pd.buffer);
		pagedirectory_num += 1;
		current_pagedirectory = page_num;
		page_num ++;
//		cout << "&" << current_pagedirectory <<"&"<< pd.next_page<<fileHandle.getNumberOfPages()<<endl;
//		cout <<"1:"<< pagedirectory_num << current_pagedirectory << page_num << endl;
	}
//	cout<<"3";
//	cout<<"4";
	if(page_num==pagedirectory_num){
//		cout<<"5";
		Page page;
		Slot slot;
		slot.record_offset = 0;
		slot.record_lenght = record.total_size;
		rid.pageNum = page_num;
		rid.slotNum = page.slot_directory.size();
		page.slot_directory.push_back(slot);
		record.rid = rid;
		page.records.push_back(record);
		page.free_ptr += record.total_size;
		page.encodePage();
//		cout << *(int*)(page.buffer+4088) << endl;

		fileHandle.appendPage(page.buffer);
		PageDirectorySlot pagedirectoryslot;
		pagedirectoryslot.free_byte = PAGE_SIZE-record.total_size-2*sizeof(int)-sizeof(Slot);
		pagedirectoryslot.page_id = 1;
		pd.num_page = 1;
		pd.pds.push_back(pagedirectoryslot);
		pd.encodePageDirectory();
		fileHandle.writePage(current_pagedirectory,pd.buffer);

//		free(&page);
//		free(&record);
//		free(&pd);
	}
	else{
//		cout << "Insert multiple record" << endl;
//		cout << "pds_size" << "--" << pd.pds.size()<< endl;
		int find = 0;
		char* tmp = (char*)malloc(PAGE_SIZE);
		fileHandle.readPage(0,tmp);
		PageDirectory tmp_pd(tmp);
		int next_page = 0;
//		cout << "before while" << endl;
//				cout <<next_page<< endl;

		while(1){
			int i=0;
//			cout << tmp_pd.pds.size() << endl;
//			cout << "--" << tmp_pd.next_page;
//			cout << next_page << endl;
//			cout << "start for" << endl;
//			cout << page_num << endl;
//			cout << "start" << tmp_pd.next_page<< endl;
			for(i=0; i< tmp_pd.pds.size(); i++){
				if(record.total_size+sizeof(Slot) <= tmp_pd.pds[i].free_byte){
	//				cout << i<< "record_size" << record.total_size<<"free_byte" << pd.pds[i].free_byte << endl;
//					cout << "*" << endl;
					char* page_buffer = (char*)malloc(PAGE_SIZE);
					fileHandle.readPage(tmp_pd.pds[i].page_id,page_buffer);
					Page page(page_buffer,recordDescriptor);
					Slot slot;
					slot.record_offset = page.free_ptr;
					slot.record_lenght = record.total_size;
					rid.pageNum = tmp_pd.pds[i].page_id;
					rid.slotNum = page.slot_directory.size();
					page.slot_directory.push_back(slot);
					record.rid = rid;
					page.records.push_back(record);
					page.free_ptr += record.total_size;
					page.encodePage();

					fileHandle.writePage(tmp_pd.pds[i].page_id,page.buffer);
					tmp_pd.pds[i].free_byte -= record.total_size+sizeof(Slot);
	//				cout << "record_size" << record.total_size<<"free_byte" << pd.pds[i].free_byte << endl;
	//				cout << current_pagedirectory << endl;
					tmp_pd.encodePageDirectory();
					fileHandle.writePage(next_page,tmp_pd.buffer);
	//				free(&page);
	//				free(&record);
	//				free(&pd);
					find = 1;
					break;
				}
			}
//			cout << "end for" << endl;
//			cout << "end" << tmp_pd.next_page << endl;
			if(find ==1)
				break;
			if(tmp_pd.next_page==-1)
				break;
			else{
				next_page = tmp_pd.next_page;
//				cout << "###" << next_page << endl;
//				free(tmp);
	//			char* tmp = (char*)malloc(PAGE_SIZE);
				fileHandle.readPage(next_page,tmp);
//				free(&tmp_pd);

				PageDirectory tmp_pd1(tmp);
				tmp_pd = tmp_pd1;

			}
//
		}

		if(find == 0)
		{
//			cout << "%%" << endl;
//			cout << "New page" << endl;
//			cout << i-1<< "free_byte" << pd.pds[i-1].free_byte << endl;
			Page page;
			Slot slot;
			slot.record_offset = 0;
			slot.record_lenght = record.total_size;
			rid.pageNum = page_num;
			rid.slotNum = page.slot_directory.size();
			page.slot_directory.push_back(slot);
			record.rid = rid;
			page.records.push_back(record);
			page.free_ptr += record.total_size;
			page.encodePage();
	//		cout << *(int*)(page.buffer+4088) << endl;

			fileHandle.appendPage(page.buffer);
			PageDirectorySlot pagedirectoryslot;
			pagedirectoryslot.free_byte = PAGE_SIZE-record.total_size-2*sizeof(int)-sizeof(Slot);
			pagedirectoryslot.page_id = page_num;
			pd.num_page += 1;
//			cout <<"$"<< pd.num_page << endl;
			pd.pds.push_back(pagedirectoryslot);
//			cout << "**"<< pagedirectoryslot.free_byte << endl;
			pd.encodePageDirectory();
			fileHandle.writePage(current_pagedirectory,pd.buffer);
		}
//		cout << "end else" << endl;
	}

	free(pd_tmp);
//	cout<<"7";
    return 0;
}

RC RecordBasedFileManager::readRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid, void *data) {
//	cout << "ReadRecord" << rid.pageNum << endl;
	char* page_buffer = (char*)malloc(PAGE_SIZE);
	fileHandle.readPage(rid.pageNum,page_buffer);

//	cout << "*"<< endl;
	Page page(page_buffer,recordDescriptor);
	Slot slot;
	memcpy(&slot,&(page.slot_directory[rid.slotNum]),sizeof(Slot));

	Record record(recordDescriptor,page.buffer,slot);
	record.decodeRecord(data);

	free(page_buffer);

    return 0;
}

RC RecordBasedFileManager::printRecord(const vector<Attribute> &recordDescriptor, const void *data) {
//	cout << "PrintRecord" << endl;
	int offset = 0;
	int null_bit = ceil(1.0*recordDescriptor.size()/8);
	unsigned char* null_indicator = (unsigned char*)malloc(null_bit);
	memcpy(null_indicator,data,null_bit);
	if(null_indicator==NULL){
		perror("Read null bit error!");
		return -1;
	}

	offset = null_bit;
	int null_byte_num;
	int null_bit_num;
	for(int i=0;i<recordDescriptor.size();i++){
		null_byte_num = i/8;
		null_bit_num = i%8;
		if(null_indicator[null_byte_num] & (1 << (7-null_bit_num))){
			cout << recordDescriptor[i].name << ":" << NULL << endl;
		}
		else{
			switch(recordDescriptor[i].type){
			case TypeInt:
			{
				int int_attr;
				memcpy(&int_attr,(char*)data+offset,recordDescriptor[i].length);
				offset += recordDescriptor[i].length;
				cout << recordDescriptor[i].name << ":" << int_attr << endl;
				break;
			}
			case TypeReal:
			{
				float real_attr;
				memcpy(&real_attr,(char*)data+offset,recordDescriptor[i].length);
				offset += recordDescriptor[i].length;
				cout << recordDescriptor[i].name << ":" << real_attr << endl;
				break;
			}
			case TypeVarChar:
			{
				int attr_len;
				memcpy(&attr_len,(char*)data+offset,4*sizeof(char));
				offset += 4*sizeof(char);
				char* char_attr = (char*)malloc(attr_len);
				memcpy(char_attr,(char*)data+offset,attr_len);
				offset += attr_len;
				cout << recordDescriptor[i].name << ":" << char_attr << endl;
				free(char_attr);
				break;
			}
			}
		}
	}

	free(null_indicator);
    return 0;
}


//Field::Field(){
//}
//Field::~Field(){
//}

Record::Record(){
//	cout << "Record1" << endl;
}
Record::~Record(){
}
Record::Record(const vector<Attribute> &recordDescriptor, const void *data):field_size(0),total_size(0){
//	cout << "Record2" << endl;
	record_Descriptor = recordDescriptor;
	int offset = 0;
	null_bit = ceil(1.0*recordDescriptor.size()/8);
	null_indicator = (unsigned char*)malloc(null_bit);
	memcpy(null_indicator,data,null_bit);
	if(null_indicator==NULL){
		perror("Read null bit error!");
	}

	offset = null_bit;
	int null_byte_num;
	int null_bit_num;
	for(int i=0;i<recordDescriptor.size();i++){
		null_byte_num = i/8;
		null_bit_num = i%8;
		if(null_indicator[null_byte_num] & (1 << (7-null_bit_num))){
//			cout << recordDescriptor[i].name << ":" << NULL << endl;
//			offset += recordDescriptor[i].length;
			field_size += recordDescriptor[i].length;
		}
		else{
			switch(recordDescriptor[i].type){
			case TypeInt:
			{
				IntField int_field;
				int_field.attr = recordDescriptor[i];
				memcpy(&int_field.int_value,(char*)data+offset,recordDescriptor[i].length);
				offset += recordDescriptor[i].length;
				field_size += recordDescriptor[i].length;
//				cout << recordDescriptor[i].name << ":" << int_field.value << endl;
				fields.push_back(int_field);
//				free(&int_field);
				break;
			}
			case TypeReal:
			{
				RealField real_field;
				real_field.attr = recordDescriptor[i];
				memcpy(&real_field.real_value,(char*)data+offset,recordDescriptor[i].length);
				offset += recordDescriptor[i].length;
				field_size += recordDescriptor[i].length;
//				cout << recordDescriptor[i].name << ":" << real_field.value << endl;
				fields.push_back(real_field);
//				free(&real_field);
				break;
			}
			case TypeVarChar:
			{
				VarCharField varchar_field;
				varchar_field.attr = recordDescriptor[i];
				int attr_len;
				memcpy(&attr_len,(char*)data+offset,4*sizeof(char));
				varchar_field.attr.length = attr_len;
				offset += 4*sizeof(char);
				varchar_field.varchar_value = (char*)malloc(attr_len); // Why need a middle value?
				memcpy(varchar_field.varchar_value,(char*)data+offset,attr_len);
//				varchar_field.varchar_value = attr_value;
				offset += attr_len;
				field_size += attr_len;
//				cout << recordDescriptor[i].name << ":" << varchar_field.value << endl;
				fields.push_back(varchar_field);
//				free(attr_value);
				break;
			}
			}
		}
		total_size = null_bit + field_size + 4*record_Descriptor.size();
//		cout<< "total" << total_size << endl;
	}

}
Record::Record(const vector<Attribute> &recordDescriptor,char* page_buffer, Slot slot):field_size(0),total_size(0){
//	cout << "Record3" << endl;
//	cout << *(float*)(page_buffer+29) << endl;
	record_Descriptor = recordDescriptor;
	buffer = (char*)malloc(slot.record_lenght);
//	cout << slot.record_lenght << endl;

	memcpy(buffer,page_buffer+slot.record_offset,slot.record_lenght);

	int offset = 0;
	null_bit = ceil(1.0*recordDescriptor.size()/8);
	null_indicator = (unsigned char*)malloc(null_bit);
	memcpy(null_indicator,buffer,null_bit);
	if(null_indicator==NULL){
		perror("Read null bit error!");
	}

	offset = null_bit;
	offset += recordDescriptor.size()*sizeof(int);
	int null_byte_num;
	int null_bit_num;
	for(int i=0;i<recordDescriptor.size();i++){

		null_byte_num = i/8;
		null_bit_num = i%8;
//		cout << "offset" << offset << endl;
		if(null_indicator[null_byte_num] & (1 << (7-null_bit_num))){
//			cout << recordDescriptor[i].name << ":" << NULL << endl;
			offset += recordDescriptor[i].length;
			field_size += recordDescriptor[i].length;
		}
		else{
			switch(recordDescriptor[i].type){
			case TypeInt:
			{
				IntField int_field;
				int_field.attr = recordDescriptor[i];
				memcpy(&int_field.int_value,buffer+*(int*)(buffer+null_bit+4*i),recordDescriptor[i].length);
				offset += recordDescriptor[i].length;
				field_size += recordDescriptor[i].length;
//				cout << recordDescriptor[i].name << ":" << int_field.value << endl;
				fields.push_back(int_field);
//				free(&int_field);
				break;
			}
			case TypeReal:
			{

				RealField real_field;
				real_field.attr = recordDescriptor[i];
				memcpy(&real_field.real_value,buffer+*(int*)(buffer+null_bit+4*i),recordDescriptor[i].length);
				offset += recordDescriptor[i].length;
				field_size += recordDescriptor[i].length;
//				cout << recordDescriptor[i].name << ":" << real_field.value << endl;
				fields.push_back(real_field);
//				free(&real_field);
				break;
			}
			case TypeVarChar:
			{
//				cout <<"*"<< endl;

				VarCharField varchar_field;
				varchar_field.attr = recordDescriptor[i];
				int attr_len;
				if(i==recordDescriptor.size()-1)
					attr_len = slot.record_lenght - offset;
				else
					attr_len = *(int*)(buffer+null_bit+4*i+4) - offset;
//				cout << *(int*)(buffer+null_bit+4*i+4) -offset << attr_len << endl;
				varchar_field.attr.length = attr_len;
//				char * attr_value = (char*)malloc(attr_len); // Why need a middle value?
				varchar_field.varchar_value = (char*)malloc(attr_len);
//				char tmp[attr_len+1];
				memcpy(varchar_field.varchar_value,buffer+*(int*)(buffer+null_bit+4*i),attr_len);
//				memcpy(tmp,buffer+*(buffer+null_bit+i),attr_len);
//				tmp[attr_len]='\0';
//				cout << "tmpp" << *(varchar_field.varchar_value+1) << endl;
//				varchar_field.varchar_value = attr_value;
				offset += attr_len;
				field_size += attr_len;
//				cout << recordDescriptor[i].name << ":" << varchar_field.value << endl;
				fields.push_back(varchar_field);
//				free(attr_value);


				break;
			}
			}

		}

		total_size = null_bit + field_size + 4*record_Descriptor.size();
	}
}



RC Record::encodeRecord(){
//	cout << "encodeRecord" << endl;
	int offset = 0;
//	cout << offset << endl;

	if(sizeof(*buffer)!=0)
		buffer = (char*)malloc(total_size);

	memcpy(buffer+offset,null_indicator,null_bit);

	offset += null_bit;
//	cout << offset << endl;
	offset += 4*record_Descriptor.size();
//	cout << offset << endl;
	int null_byte_num;
	int null_bit_num;
	int null_num = 0;
//	cout << "##" << record_Descriptor.size() << endl;

	for(int i=0;i<record_Descriptor.size();i++){
		null_byte_num = i/8;
		null_bit_num = i%8;
//		cout << offset << endl;
		if(null_indicator[null_byte_num] & (1 << (7-null_bit_num))){
		//			cout << recordDescriptor[i].name << ":" << NULL << endl;
			null_num++;
			offset += record_Descriptor[i].length;
//			cout << "**" << endl;
		}
		else{
//			cout << "why" << i-null_num << endl;

		switch(fields[i-null_num].attr.type){
		case TypeInt:
		{
			memcpy(buffer+offset,&(fields[i-null_num].int_value),fields[i-null_num].attr.length);
//			buffer[null_bit+i] = offset;
			memcpy(buffer+null_bit+4*i,&offset,sizeof(int));
			offset += fields[i-null_num].attr.length;
//			cout << offset << endl;
			break;
		}
		case TypeReal:
		{
//			cout << "real" << fields[i-null_num].real_value << endl;
			memcpy(buffer+offset,&(fields[i-null_num].real_value),fields[i-null_num].attr.length);
//			cout << *(float*)(buffer+offset) << endl;
//			buffer[null_bit+i] = offset;
			memcpy(buffer+null_bit+4*i,&offset,sizeof(int));
			offset += fields[i-null_num].attr.length;
//			cout << offset << endl;
			break;
		}
		case TypeVarChar:
		{
			memcpy(buffer+offset,fields[i-null_num].varchar_value,fields[i-null_num].attr.length);
//			buffer[null_bit+i] = offset;
			memcpy(buffer+null_bit+4*i,&offset,sizeof(int));
			offset += fields[i-null_num].attr.length;
//			cout << offset << endl;
			break;
		}
		}
		}
	}

//	cout << offset << endl;
//	cout << *buffer << endl;

	return 0;
}

RC Record::decodeRecord(void* return_data){
//	cout << "decodeRecord" << endl;
	int offset = 0;
	memcpy((char*)return_data+offset,null_indicator,null_bit);
	offset += null_bit;
	for(int i=0; i<fields.size(); i++){
		switch(fields[i].attr.type){
		case TypeInt:
		{
			memcpy((char*)return_data+offset,&fields[i].int_value,sizeof(int));
			offset += sizeof(int);
			break;
		}
		case TypeReal:
		{
			memcpy((char*)return_data+offset,&fields[i].real_value,sizeof(float));
			offset += sizeof(float);
			break;
		}
		case TypeVarChar:
		{
			memcpy((char*)return_data+offset,&fields[i].attr.length,sizeof(int));
			offset += sizeof(int);
			memcpy((char*)return_data+offset,fields[i].varchar_value,fields[i].attr.length);
			offset += fields[i].attr.length;
			break;
		}
	}
	}
//	cout << "returndatalength" << offset << endl;
	return 0;
}

Page::Page(){
//	cout << "Page1" << endl;
	buffer = (char*)malloc(PAGE_SIZE);
	free_ptr = 0;

};
Page::~Page(){
}

Page::Page(char* tmp,vector<Attribute> record_Descriptor){
//	cout << "Page2" << endl;
	buffer = (char*)malloc(PAGE_SIZE);
	memcpy(buffer,tmp,PAGE_SIZE);
	int offset = PAGE_SIZE-sizeof(int);
	memcpy(&free_ptr, buffer+offset,sizeof(int));
	offset -= sizeof(int);
	int rec_num;
	memcpy(&rec_num, buffer+offset,sizeof(int));
	for(int i=0;i<rec_num;i++){
		Slot slot;
		offset -= sizeof(Slot);
//		cout <<"haha"<< *(int*)(buffer+4084) << endl;

		memcpy(&slot,buffer+offset,sizeof(Slot));
		slot_directory.push_back(slot);
//		void * data = malloc(slot.record_lenght);
//		memcpy(data,buffer+slot.record_offset,slot.record_lenght);
		Record record(record_Descriptor,buffer,slot);
//		record.total_size = slot.record_lenght;
		records.push_back(record);
//		cout << slot.record_lenght << endl;

	}

}

RC Page::encodePage(){
//	cout << "encodePage" << endl;
	int offset = PAGE_SIZE-sizeof(int);
	memcpy(buffer+offset,&free_ptr,sizeof(int));
	offset -= sizeof(int);
	int rec_num = records.size();
	memcpy(buffer+offset,&rec_num,sizeof(int));
	for(int i=0; i<slot_directory.size();i++){
		records[i].encodeRecord();
		memcpy(buffer+slot_directory[i].record_offset,records[i].buffer,records[i].total_size);
		if(slot_directory[i].record_offset+records[i].total_size > free_ptr)
			free_ptr = slot_directory[i].record_offset+records[i].total_size;

		offset -= sizeof(Slot);
		memcpy(buffer+offset,&slot_directory[i],sizeof(Slot));
	}

	return 0;
}

PageDirectory::PageDirectory(char* tmp){
//	cout << "PageDirectory2" << endl;
	buffer = (char*)malloc(PAGE_SIZE);
	memcpy(buffer,tmp,PAGE_SIZE);
	int offset = 0;
	memcpy(&next_page,buffer+offset,sizeof(int));
	offset += sizeof(int);
	memcpy(&num_page,buffer+offset,sizeof(int));
	offset += sizeof(int);
	for(int i=0;i< num_page;i++){
		PageDirectorySlot pagedirectoryslot;
		memcpy(&pagedirectoryslot,buffer+offset,sizeof(PageDirectorySlot));
		offset += sizeof(PageDirectorySlot);
		pds.push_back(pagedirectoryslot);
	}
}

RC PageDirectory::encodePageDirectory(){

//	cout << "encodePageDirectory" << endl;
	int offset = 0;
	memcpy(buffer+offset,&next_page,sizeof(int));
	offset += sizeof(int);
	memcpy(buffer+offset,&num_page,sizeof(int));
	offset += sizeof(int);
	for(int i=0; i<pds.size(); i++){
		memcpy(buffer+offset,&pds[i],sizeof(PageDirectorySlot));
		offset += sizeof(PageDirectorySlot);
	}
	return 0;
}







