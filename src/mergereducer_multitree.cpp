#include "mergereducer_multitree.h"


template <class K, class V>
SortReduceReducer::MergeReducer_MultiTree<K,V>::MergeReducer_MultiTree(V (*update)(V,V), std::string temp_directory, std::string filename, bool verbose) {
	this->m_done = false;
	this->m_started = false;

	this->mp_update = update;

	this->mp_stream_file_reader = new StreamFileReader(temp_directory, verbose);
	this->mp_reducer_node = new ReducerNode<K,V>(update, temp_directory, filename);

	this->mvv_tree_nodes.push_back(std::vector<BlockSource<K,V>*>());
}

template <class K, class V>
SortReduceReducer::MergeReducer_MultiTree<K,V>::~MergeReducer_MultiTree() {
	delete mp_reducer_node;

	for ( int i = 0; i < mv_tree_nodes_seq.size(); i++ ) {
		delete mv_tree_nodes_seq[i];
	}

}

template <class K, class V>
void
SortReduceReducer::MergeReducer_MultiTree<K,V>::PutBlock(SortReduceTypes::Block block) {
	fprintf( stderr, "MergeReducer_MultiTree not used with blocks...yet\n" );
}

template <class K, class V>
void
SortReduceReducer::MergeReducer_MultiTree<K,V>::PutFile(SortReduceTypes::File* file) {
	if ( this->m_started ) {
		fprintf(stderr, "Attempting to add data source to started reducer\n" );
		return;
	}
	mp_stream_file_reader->PutFile(file);

	int cur_count = mv_file_reader.size();
	FileReaderNode<K,V>* reader = new FileReaderNode<K,V>(mp_stream_file_reader, cur_count);
	mv_file_reader.push_back(reader);
	mvv_tree_nodes[0].push_back(reader);
}

template <class K, class V>
void
SortReduceReducer::MergeReducer_MultiTree<K,V>::Start() {
	this->m_started = true;

	size_t input_count = mv_file_reader.size();
	printf( "MergeReducer_MultiTree started with %lu files\n", input_count ); fflush(stdout);




	int cur_level = 0;
	int cur_level_count = input_count;

	while (cur_level_count > 1) {
		mvv_tree_nodes.push_back(std::vector<BlockSource<K,V>*>());

		for ( int i = 0; i < cur_level_count/2; i++ ) {
			MergerNode<K,V>* merger = new MergerNode<K,V>(1024*1024, 4);
			merger->AddSource(mvv_tree_nodes[cur_level][i*2]);
			merger->AddSource(mvv_tree_nodes[cur_level][i*2+1]);
			merger->Start();
			mvv_tree_nodes[cur_level+1].push_back(merger);

			mv_tree_nodes_seq.push_back(merger);
		}
		if ( cur_level_count%2 == 1 ) {
			mvv_tree_nodes[cur_level+1].push_back(mvv_tree_nodes[cur_level][cur_level_count-1]);
		}

		cur_level_count = mvv_tree_nodes[cur_level+1].size();
		cur_level++;
	}

	mp_reducer_node->SetSource(mvv_tree_nodes[cur_level][0]);
}

template <class K, class V>
bool
SortReduceReducer::MergeReducer_MultiTree<K,V>::IsDone() {
	return mp_reducer_node->IsDone();
}



TEMPLATE_EXPLICIT_INSTANTIATION(SortReduceReducer::MergeReducer_MultiTree)
