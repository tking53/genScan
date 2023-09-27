#ifndef __DATA_BUFFER_HPP__
#define __DATA_BUFFER_HPP__

#include <fstream>

void set_char_array(const std::string &input_, char *arr_,const unsigned int &size_) {
	size_t size_to_copy = size_;
	if (size_ > input_.size()){ 
		size_to_copy = input_.size(); 
	}
	for (unsigned int i = 0; i < size_; i++){
		if(i < size_to_copy){ 
			arr_[i] = input_[i]; 
		}else{ 
			arr_[i] = ' '; 
		}
	}
	arr_[size_] = '\0';
}



class DataBuffer{
	public:
		unsigned int GetBufferType() const;
		unsigned int GetBufferSize() const;
		unsigned int GetBufferEndFlag() const;

		bool IsDebugMode() const;
		void SetDebugMode(bool);
		bool ReadHeader(std::ifstream&) const;

	protected:
		unsigned int bufftype;
		unsigned int buffsize;
		unsigned int buffend;
		unsigned int zero;
		bool debug_mode;

		DataBuffer(unsigned int, unsigned int);
		DataBuffer(unsigned int, unsigned int, unsigned int);
		virtual bool Write(std::ofstream&);
		virtual bool Read(std::ifstream&);
		virtual void Reset();
};

#endif
