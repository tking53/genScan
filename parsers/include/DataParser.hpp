#ifndef __DATA_PARSER_HPP__
#define __DATA_PARSER_HPP__

class DataParser{
	public:
		enum DataFileType{
			Unknown,
			CAEN_ROOT,
			CAEN_BIN,
			LDF,
			PLD,
			EVT
		};
		static DataParser* Get();
		static DataParser* Get(DataFileType);
	private:
		DataFileType DataType;

		DataParser();
		DataParser(DataFileType);
		static DataParser* instance;
};

#endif
