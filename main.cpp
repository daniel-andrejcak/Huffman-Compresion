#include <algorithm>
#include <bitset>
#include <fstream>
#include <iostream>
#include <memory>
#include <set> 
#include <string>
#include <unordered_map>
#include <vector>
#include <queue>

class Node {
public:
	Node() {};
	explicit Node(uint64_t freq) : freq(freq) {};
	explicit Node(unsigned char c) : c(c) {};

	Node(char c, uint64_t freq) : c(c), freq(freq) { hasChar = true; };

	~Node() {};

	unsigned char c = 0;
	bool hasChar = false;
	uint64_t freq = 0;
	std::string path;
	std::shared_ptr<Node> left = nullptr;
	std::shared_ptr<Node> right = nullptr;
};

struct CompareNodesFreq {
	bool operator()(const std::shared_ptr<Node>& lhs, const std::shared_ptr<Node>& rhs) const {
		return lhs->freq > rhs->freq;
	}
};

struct CompareNodesChar {
	bool operator()(const Node& lhs, const Node& rhs) const {
		return lhs.c < rhs.c;
	}
};


std::shared_ptr<Node> buildHuffmanTree(const std::unordered_map<unsigned char, int>& frequencyMap) {
	std::priority_queue<std::shared_ptr<Node>, std::vector<std::shared_ptr<Node>>, CompareNodesFreq> queue;

	for (auto& charFreqPair: frequencyMap)
	{
		queue.push(std::make_shared<Node>(charFreqPair.first, charFreqPair.second));
	}

	if (queue.size() == 1)
	{
		auto root = std::make_shared<Node>();
		root->left = queue.top();
		queue.pop();

		return root;
	}

	while (queue.size() > 1) {
		auto left = queue.top();
		queue.pop();
		
		auto right = queue.top();
		queue.pop();

		auto combined = std::make_shared<Node>(left->freq + right->freq);
		combined->left = left;
		combined->right = right;

		queue.push(combined);
	}

	auto root = queue.top();

	return root;

}


std::unordered_map<unsigned char, int> buildFrequencyMap(const std::vector<unsigned char>& characters) {
	std::unordered_map<unsigned char, int> frequencyMap;

	for (auto& c : characters)
	{
		frequencyMap[c] += 1;
	}

	return frequencyMap;
}


void treeTraversal(std::shared_ptr<Node>& node, std::set<Node, CompareNodesChar>& charactersPath, std::string path = "") {
	if (node.get()->hasChar)
	{
		node.get()->path = path;
		charactersPath.insert(*node);
		return;
	}
	
	if (node->left != nullptr)	
		treeTraversal(node->left, charactersPath, (path + '1'));

	if (node->right != nullptr)	
		treeTraversal(node->right, charactersPath, (path + '0'));
}


std::vector<unsigned char> readFile(const std::string& str) {
	std::ifstream inputFile(str, std::ios::binary);

	if (!inputFile.is_open()) {
		exit(EXIT_FAILURE);
	}

	std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(inputFile), {});

	return buffer;
}


void print(std::set<Node, CompareNodesChar> characters) {
	for (auto& node : characters)
	{
		std::cout << static_cast<int>(node.c) << ": " << node.path << std::endl;
	}
}


void compress(std::vector<unsigned char> individualBytes, std::set<Node, CompareNodesChar> characters, std::string fileName) {
	uint64_t size = 0;

	std::vector<std::string> codesForAllBytes(256);

	for (auto& character : characters)
	{
		codesForAllBytes[static_cast<size_t>(character.c)] = character.path;
	}


	std::vector<uint8_t> encodedBytes;
	std::string tempEncode;

	for (auto& byte : individualBytes)
	{
		Node temp(byte);
		auto searchNode = characters.find(temp);

		tempEncode += searchNode->path;

		while (tempEncode.size() >= 8)
		{
			std::string substring = tempEncode.substr(0, 8);

			encodedBytes.push_back(static_cast<uint8_t>(std::bitset<8>(substring).to_ulong()));

			tempEncode.erase(0, 8);
		}		
		size += searchNode->path.size();
	}


	if (!tempEncode.empty())
	{
		while (tempEncode.size() < 8)
		{
			tempEncode += '0';
		}

		encodedBytes.push_back(static_cast<uint8_t>(std::bitset<8>(tempEncode).to_ulong()));
	}


	std::ofstream outputFile(fileName, std::ios::binary);

	if (!outputFile.is_open()) {
		exit(EXIT_FAILURE);
	}

	outputFile.write(reinterpret_cast<char*>(&size), sizeof(size));

	for (auto& code : codesForAllBytes)
	{
		if (!code.empty()) 
		{
			uint8_t size = static_cast<uint8_t>(code.size());
			outputFile.write(reinterpret_cast<char*>(&size), sizeof(size));

			for (size_t i = 0; i < code.length(); i += 8) {
				std::string substring = code.substr(i, 8);

				while (substring.size() < 8)
				{
					substring += '0';
				}

				uint8_t path = static_cast<uint8_t>(std::bitset<8>(substring).to_ulong());
				outputFile.write(reinterpret_cast<char*>(&path), sizeof(path));
				  
			}
		}
		else
		{
			uint8_t noChar = 0;
			outputFile.write(reinterpret_cast<char*>(&noChar), sizeof(noChar));
		}
	}

	for (auto& code : encodedBytes)
	{
		outputFile.write(reinterpret_cast<char*>(&code), sizeof(code));
	}
}


uint64_t getSize(const std::vector<unsigned char>& bytes) {
	uint64_t size = 0;

	for (size_t i = 0; i < 8; i++)
	{
		size |= (static_cast<uint64_t>(bytes[i]) << 8*i);
	}

	return size;
}

//return paths for all characters (256) and the encoded file separated
std::pair<std::vector<std::string>, std::string> decodeBytes(const std::vector<unsigned char>& bytes) {

	std::vector<std::string> paths(256);

	uint64_t index = 0;
	uint16_t charIndex = 0;
	while (charIndex != 256)
	{
		if (bytes[index + 8] != 0)
		{
			uint8_t pathSize = bytes[index + 8];

			std::string path;

			if (pathSize <= 8)
			{
				std::bitset<8> binaryPathRepresentation(bytes[index + 8 + 1]);
				path += binaryPathRepresentation.to_string();

				index += 1;
			}
			else
			{
				for (size_t i = 1; i <= pathSize / 8 + 1; i++)
				{
					std::bitset<8> binaryPathRepresentation(bytes[index + 8 + i]);
					path += binaryPathRepresentation.to_string();
				}

				index += pathSize / 8 + 1;
			}

			path.resize(pathSize);
			paths[charIndex] = path;
		}
		

		index += 1;
		charIndex += 1;
	}

	std::string code;

	for (size_t i = index + 8; i < bytes.size(); i++)
	{
		std::bitset<8> binaryPathRepresentation(bytes[i]);
		code += binaryPathRepresentation.to_string();
	}

	std::pair<std::vector<std::string>, std::string> pathEncodePair(paths, code);
	
	return pathEncodePair;
}


std::shared_ptr<Node> insertNode(std::shared_ptr<Node> root, std::string& path, unsigned char c) {
	std::shared_ptr<Node> currentNode = root;
	for (char bit : path) {
		if (bit == '0') {
			if (currentNode->right == nullptr) {
				currentNode->right = std::make_shared<Node>();
			}
			currentNode = currentNode->right;
		}
		else if (bit == '1') {
			if (currentNode->left == nullptr) {
				currentNode->left = std::make_shared<Node>();
			}
			currentNode = currentNode->left;
		}
	}

	currentNode->c = c;
	currentNode->hasChar = true;

	return root;
}


std::shared_ptr<Node> buildHuffmanTreeFromPaths(std::vector<std::string> paths) {

	std::shared_ptr<Node> root = std::make_shared<Node>();

	for (size_t i = 0; i < 256; i++)
	{
		if (!paths[i].empty())
		{
			unsigned char c = static_cast<unsigned char>(i);
			root = insertNode(root, paths[i], c);
		}
	}

	return root;
}


void writeChar(std::shared_ptr<Node> root, std::string& code, std::ofstream& outputFile) 
{
	uint64_t index = 0;
	while (!root->hasChar)
	{
		if (code[index] == '1')
		{
			root = root->left;
		}
		else if (code[index] == '0')
		{
			root = root->right;
		}

		index += 1;
	}

	code.erase(0, index);
	
	outputFile.write(reinterpret_cast<char*>(&root->c), sizeof(char));
}

void decompress(const std::shared_ptr<Node>& root,const std::string& code, const std::string& fileName) {
	std::ofstream outputFile(fileName, std::ios::binary);

	std::shared_ptr<Node> currentNode = root;
	
	for (char bit : code) {
		if (bit == '1') {
			currentNode = currentNode->left;
		}
		else if (bit == '0') {
			currentNode = currentNode->right;
		}

		if (currentNode->hasChar) {
			outputFile.put(currentNode->c);
			currentNode = root; 
		}
	}


	outputFile.close();
}

void printTree(std::shared_ptr<Node> root, std::string path = "") {
	if (root == nullptr)
	{
		return;
	}
	if (root.get()->hasChar)
	{
		std::cout << path << root->c << std::endl;
		return;
	}

	printTree(root->left, (path + '1'));
	printTree(root->right, (path + '0'));
}

int main(int argc, char* argv[]) {

	if (argc < 3 || argc > 4)
		return EXIT_FAILURE;

	std::vector<std::string> arguments(argv, argv + argc);

	auto individualBytes = readFile(arguments[2]);


	if (arguments[1] == "--decompress")
	{
		auto size = getSize(individualBytes);

		auto pathCodePair = decodeBytes(individualBytes);

		auto root = buildHuffmanTreeFromPaths(pathCodePair.first);

		//printTree(root);

		pathCodePair.second.resize(size);

		decompress(root, pathCodePair.second, arguments[3]);
	}
	else if (arguments[1] == "--print")
	{
		if (argc != 3)
			return EXIT_FAILURE;

		auto freqMap = buildFrequencyMap(individualBytes);

		auto root = buildHuffmanTree(freqMap);

		std::set<Node, CompareNodesChar> characters;

		treeTraversal(root, characters);

		print(characters);
	}
	else if (arguments[1] == "--compress")
	{

		auto freqMap = buildFrequencyMap(individualBytes);

		auto root = buildHuffmanTree(freqMap);

		std::set<Node, CompareNodesChar> characters;

		treeTraversal(root, characters);

		compress(individualBytes, characters, arguments[3]);
	}
	else
	{
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
