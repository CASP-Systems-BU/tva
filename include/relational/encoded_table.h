#ifndef TVA_ENCODED_TABLE_H
#define TVA_ENCODED_TABLE_H

#include "encoded_column.h"
#include "shared_column.h"
#include "../service/common/runtime.h"

#include <map>
#include <memory>

namespace tva { namespace relational {

    // TODO (john): Support columns with different share types.
    /**
     * An EncodedTable is a relational table that contains encoded data organized in columns. TVA users can perform
     * vectorized secure operations directly on table columns via a Pandas-like interface, as shown below:
     *
     \verbatim
     // TVA program (executed by an untrusted party)
     t["col3"] = t["col1"] * t["col2"];
     \endverbatim
     *
     * The above line performs a secure elementwise multiplication between two columns `col1` and `col2` in table `t`
     * and stores the result in a third column `col3`.
     *
     * @tparam SharedColumn - Secret-shared column type.
     * @tparam ASharedVector - A-shared vector type.
     * @tparam BSharedVector - B-shared vector type.
     * @tparam EncodedVector - Encoded vector type.
     * @tparam DataTable - Plaintext table type.
     */
    template<typename ShareType,
            typename SharedColumn,
            typename ASharedVector,
            typename BSharedVector,
            typename EncodedVector,
            typename DataTable>
    class EncodedTable {
        // The table schema, i.e., a mapping from column names to column pointers.
        std::map<std::string, std::shared_ptr<EncodedColumn> > schema;
        // The pointers to the table columns in the order they are defined by the user.
        std::vector<std::shared_ptr<EncodedColumn> > columns;
        // Column encodings (True if B-Shared, False otherwise).
        std::vector<bool> bShare;
        // The table's name.
        std::string tableName;
        // The number of rows in the table
        int rows;

    public:
        /**
         * Constructs a table and initializes it with zeros.
         *
         * @param _tableName - The name of the table.
         * @param _columns - The column names.
         * @param _rows - The number of rows to allocate.
         */
        EncodedTable(const std::string &_tableName,
                     const std::vector<std::string> &_columns,
                     const int &_rows) :
                tableName(_tableName), rows(_rows)
        {
            addColumns(_columns, _rows);  // Allocate columns
        }

        /**
         * Constructs a table from encoded columns.
         * @param contents - The table columns.
         */
        EncodedTable(std::vector<std::shared_ptr<EncodedColumn>>&& contents) :
                columns(contents)
        {
            // Set table cardinality and metadata
            assert(columns.size()>0);
            rows = columns[0]->size();
            schema.clear();
            bShare.clear();
            for (auto &c : columns) {
                assert(rows == c->size());
                auto p = std::pair<std::string, std::shared_ptr<EncodedColumn>>(c->name, c);
                auto r = schema.insert(p);
                assert(r.second);
                bShare.push_back(isBShared(c->name));
            }
        }

        /**
         * Populates table with randomly generated values.
         */
        void populatePRandom(){
            for(int i = 0; i < columns.size(); ++i){
                if(bShare[i]) {
                    auto c = reinterpret_cast<BSharedVector*>(columns[i].get()->contents.get());
                    c->populatePRandom();
                }else{
                    auto c = reinterpret_cast<ASharedVector*>(columns[i].get()->contents.get());
                    c->populatePRandom();
                }
            }
        }

        /**
         * Returns a mutable reference to the column with the given name.
         * @param name - The name of the column.
         * @return A reference to the column (throws an error if the column is not found).
         */
        inline EncodedColumn &operator[](const std::string &name) {
            return *((EncodedColumn*)schema.at(name).get());
        }

        /**
         * Opens the encoded table to all computing parties.
         * @return The opened (plaintext) table.
         */
        DataTable open() {
            DataTable res;
            for (int i = 0; i < columns.size(); ++i) {
                if (bShare[i]) {
                    res.push_back(((BSharedVector*) (columns[i].get())->contents.get())->open());
                } else {
                    res.push_back(((ASharedVector*) (columns[i].get())->contents.get())->open());
                }
            }
            return res;
        }

        /**
         * Creates table columns based on a schema.
         * @param columns - The schema.
         * @param rows - The column length.
         */
        void addColumns(const std::vector<std::string> &columns_, const int &rows_) {
            for (auto &column : columns_) {
                addColumns(column, rows_);
            }
        }

        /**
         * Allocates a column of a given size and initializes it with zeros.
         *
         * @param column - The name of the column to allocate.
         * @param rows - The column size in number of elements.
         *
         * NOTE: The default column encoding is A-shared. To create a B-shared column, you must enclose the column's
         * name with square brackets, i.e. `column`=[name].
         */
        void addColumns(const std::string &column, const int &rows_) {
            auto v = std::unique_ptr<EncodedVector>();
            if (isBShared(column)) {
                v = std::unique_ptr<EncodedVector>(new BSharedVector(rows_));
            } else {
                v = std::unique_ptr<EncodedVector>(new ASharedVector(rows_));
            }
            // Create a column from the allocated vector
            auto c = std::shared_ptr<EncodedColumn>(new SharedColumn(std::move(v)));
            // Add column to the set of columns
            columns.push_back(c);
            // Mark column encoding
            bShare.push_back(isBShared(column));
            // Update table schema
            auto r = schema.insert(std::pair<std::string, std::shared_ptr<EncodedColumn>>(column, c));
            if (!r.second) {
                std::cerr << "Table column with name " << column << " already exists." << std::endl;
                exit(-1);
            }
        }

        /**
         * @return The table name.
         */
        std::string name() const {
            return this->tableName;
        }

        /**
         * @return The table cardinality.
         */
        int size() const {
            return rows;
        }

        /**
         * Sorts `this` table in place on the given columns.
         * @param _columns - The names of the columns to sort by.
         * @param desc - A vector of flags denoting ASC (False) or DESC (True) order for each column in `_columns`.
         *
         * NOTE: All `_columns` must be B-shared.
         */
        EncodedTable& sort(const std::vector<std::string>& _keys,
                           const std::vector<bool>& desc){
            std::vector<std::string> _schema;
            for(auto it = schema.begin(); it != schema.end(); ++it){
                _schema.push_back(it->first);
            }
            return sort(_keys, desc, _schema);
        }

        EncodedTable& sort(const std::vector<std::string>& _keys,
                  const std::vector<bool>& desc,
                  const std::vector<std::string>& to_be_sorted_columns) {
            // Table size must be a power of two
            assert(ceil(log2(size())) == floor(log2(size())));


            // Sorting keys must be B-shared columns
            std::vector<BSharedVector*> keys_vec;
            for(int i = 0; i < _keys.size(); ++i){
                assert((*this)[_keys[i]].encoding == Encoding::BShared);
                keys_vec.push_back((BSharedVector*)((*this)[_keys[i]].contents.get()));
            }


            // Now, let's get remaining data in the table
            std::vector<ASharedVector*> data_a;
            std::vector<BSharedVector*> data_b;
            for(auto it = schema.begin(); it != schema.end(); ++it){

                if (std::find(_keys.begin(), _keys.end(), it->first) == _keys.end()
                && std::find(to_be_sorted_columns.begin(), to_be_sorted_columns.end(), it->first) != to_be_sorted_columns.end()) {
                    // std::cout << it->first << std::endl;
                    if (it->second->encoding == Encoding::AShared) {
                        data_a.push_back((ASharedVector*)(it->second->contents.get()));
                    } else if (it->second->encoding == Encoding::BShared) {
                        data_b.push_back((BSharedVector*)(it->second->contents.get()));
                    }
                }
            }

            operators::bitonic_sort(keys_vec, data_a, data_b, desc);

            return *this;
        }

        EncodedTable &convert_a2b(const std::string &input_a,
                                  const std::string &output_b) {
            *((BSharedVector *) (*this)[output_b].contents.get()) =
                    ((ASharedVector *) (*this)[input_a].contents.get())->a2b();

            return *this;
        }

        EncodedTable &convert_b2a_bit(const std::string &input_b,
                                  const std::string &output_a) {
            *((ASharedVector *) (*this)[output_a].contents.get()) =
                    ((BSharedVector *) (*this)[input_b].contents.get())->b2a_bit();

            return *this;
        }



        // TODO: add mask operator
        // TODO: masking oeprators in MPC works like filter operator in sql queries.
        // TODO: add selection bits for the odd even aggregation
        // TODO: for gap window, we do not actually needs to use max aggregation funciton 
        //  but we can use greater than zero aggregation function.
        EncodedTable& odd_even_aggregation(const std::vector<std::string>& _keys,
                                  const std::vector<std::string>& _data,
                                  const std::vector<std::string>& _res,
                                  const operators::AggregationType& _aggregation_type,
                                  const bool& _reverse = false,
                                  const bool& _do_sorting = false){

            if(_do_sorting){
                this->sort(_keys, std::vector<bool>(_keys.size(), false));
            }

            // Create a vector that has the BSharedVector for keys
            std::vector<BSharedVector*> keys_vec;
            for(int i = 0; i < _keys.size(); ++i){
                assert((*this)[_keys[i]].encoding == Encoding::BShared);
                keys_vec.push_back((BSharedVector*)((*this)[_keys[i]].contents.get()));
            }


            if(_aggregation_type == operators::AggregationType::SUM){
                assert(_data.size() == _res.size());

                std::vector<ASharedVector*> data_vec;
                std::vector<ASharedVector*> res_vec;
                for(int i = 0; i < _data.size(); ++i){
                    assert((*this)[_data[i]].encoding == Encoding::AShared);
                    data_vec.push_back((ASharedVector*)((*this)[_data[i]].contents.get()));

                    assert((*this)[_res[i]].encoding == Encoding::AShared);
                    res_vec.push_back((ASharedVector*)((*this)[_res[i]].contents.get()));
                }


                operators::odd_even_aggregation(keys_vec, data_vec, res_vec, &operators::sum_aggregation, _reverse);

            } else if(_aggregation_type == operators::AggregationType::MAX){
                assert(_data.size() == _res.size());

                std::vector<BSharedVector*> data_vec;
                std::vector<BSharedVector*> res_vec;
                for(int i = 0; i < _data.size(); ++i){
                    assert((*this)[_data[i]].encoding == Encoding::BShared);
                    data_vec.push_back((BSharedVector*)((*this)[_data[i]].contents.get()));

                    assert((*this)[_res[i]].encoding == Encoding::BShared);
                    res_vec.push_back((BSharedVector*)((*this)[_res[i]].contents.get()));
                }

                operators::odd_even_aggregation(keys_vec, data_vec, res_vec, &operators::max_aggregation, _reverse);

            }else if(_aggregation_type == operators::AggregationType::MIN){
                assert(_data.size() == _res.size());

                std::vector<BSharedVector*> data_vec;
                std::vector<BSharedVector*> res_vec;
                for(int i = 0; i < _data.size(); ++i){
                    assert((*this)[_data[i]].encoding == Encoding::BShared);
                    data_vec.push_back((BSharedVector*)((*this)[_data[i]].contents.get()));

                    assert((*this)[_res[i]].encoding == Encoding::BShared);
                    res_vec.push_back((BSharedVector*)((*this)[_res[i]].contents.get()));
                }

                operators::odd_even_aggregation(keys_vec, data_vec, res_vec, &operators::min_aggregation, _reverse);

            } else {
                std::cerr << "Unidentified Aggregation Type for Odd Even Aggregation" << std::endl;
                exit(-1);
            }

            return *this;
        }

        EncodedTable& distinct(const std::vector<std::string>& _keys,
                                  const std::string& _res){

            // Create a vector that has the BSharedVector for keys
            std::vector<BSharedVector*> keys_vec;
            for(int i = 0; i < _keys.size(); ++i){
                assert((*this)[_keys[i]].encoding == Encoding::BShared);
                keys_vec.push_back((BSharedVector*)((*this)[_keys[i]].contents.get()));
            }

            BSharedVector* res_ptr = (BSharedVector*)((*this)[_res].contents.get());

            operators::distinct(keys_vec, res_ptr);

            return *this;
        }

        EncodedTable& tumbling_window(const std::string & _time_a,
                              const ShareType& window_size,
                              const std::string& _res){

            assert((*this)[_time_a].encoding == Encoding::AShared);
            ASharedVector* key_ptr = (ASharedVector*)((*this)[_time_a].contents.get());

            assert((*this)[_res].encoding == Encoding::AShared);
            ASharedVector* res_ptr = (ASharedVector*)((*this)[_res].contents.get());

            operators::tumbling_window(key_ptr, window_size, res_ptr);

            return *this;
        }


        EncodedTable& gap_session_window(const std::vector<std::string>& _keys,
                                const std::string &_time_a,
                                const std::string &_time_b,
                                const std::string &_window_id,
                                const int &_gap,
                                const bool& _do_sorting = true) {
            if (_do_sorting){
                std::vector<std::string> sorting_attrs;
                sorting_attrs.push_back(_time_b);
                for(int i = 0; i < _keys.size(); ++i){
                    if(_keys[i] != _time_b){
                        sorting_attrs.push_back(_keys[i]);
                    }
                }

                std::vector<bool> sorting_direction(sorting_attrs.size(), false);
                this->sort(sorting_attrs, sorting_direction);
            }

            std::vector<BSharedVector*> keys_vec;
            for(int i = 0; i < _keys.size(); ++i){
                assert((*this)[_keys[i]].encoding == Encoding::BShared);
                keys_vec.push_back((BSharedVector*)((*this)[_keys[i]].contents.get()));
            }

            assert((*this)[_time_a].encoding == Encoding::AShared);
            ASharedVector* time_a = (ASharedVector*)((*this)[_time_a].contents.get());

            assert((*this)[_time_b].encoding == Encoding::BShared);
            BSharedVector* time_b = (BSharedVector*)((*this)[_time_b].contents.get());

            assert((*this)[_window_id].encoding == Encoding::BShared);
            BSharedVector* window_id = (BSharedVector*)((*this)[_window_id].contents.get());

            operators::gap_session_window(keys_vec, time_a, time_b, window_id, _gap);

            return *this;
        }

        // TODO: check sorting on only distinct keys + time_b
        EncodedTable& threshold_session_window(const std::vector<std::string>& _keys,
                                      const std::string &_function_res,
                                      const std::string &_time_b,
                                      const std::string &_window_id,
                                      const int &_threshold,
                                      const bool _do_sorting = true) {

            if (_do_sorting){
                std::vector<std::string> sorting_attrs;
                sorting_attrs.push_back(_time_b);
                for(int i = 0; i < _keys.size(); ++i){
                    if(_keys[i] != _time_b){
                        sorting_attrs.push_back(_keys[i]);
                    }
                }

                std::vector<bool> sorting_direction(sorting_attrs.size(), false);
                this->sort(sorting_attrs, sorting_direction);
            }

            std::vector<BSharedVector*> keys_vec;
            for(int i = 0; i < _keys.size(); ++i){
                assert((*this)[_keys[i]].encoding == Encoding::BShared);
                keys_vec.push_back((BSharedVector*)((*this)[_keys[i]].contents.get()));
            }

            assert((*this)[_function_res].encoding == Encoding::BShared);
            BSharedVector* function_res = (BSharedVector*)((*this)[_function_res].contents.get());

            assert((*this)[_time_b].encoding == Encoding::BShared);
            BSharedVector* time_b = (BSharedVector*)((*this)[_time_b].contents.get());

            assert((*this)[_window_id].encoding == Encoding::BShared);
            BSharedVector* window_id = (BSharedVector*)((*this)[_window_id].contents.get());

            operators::threshold_session_window(keys_vec, function_res, time_b, window_id, _threshold);

            return *this;
        }



        /**
         * Checks if a column with name `name` contains boolean shares.
         * @param name - The name of the column.
         * @return True if the column is B-shared, False otherwise.
         *
         * NOTE: Names of columns that include boolean shares are within square brackets, i.e., `[column_name]`.
         */
        static bool isBShared(const std::string name) {
            return name.find("[") != std::string::npos;
        }

        /**
         * Secret-shares the plaintext columns according to the encodings in the `schema`.
         * @tparam T - The plaintext data type.
         * @tparam R - The replication factor of the secret-shared columns.
         * @param columns - The plaintext columns.
         * @param schema - The encoded column names.
         * @return A vector of EncodedColumns that can be used to instantiate an EncodedTable.
         *
         * NOTE: This method can be used to construct an EncodedTable from a plaintext one as follows:
         * EncodedTable t = secret_share(plaintext_table, schema);
         */
       template <typename T, int R>
       static std::vector<std::shared_ptr<EncodedColumn>> secret_share(const std::vector<tva::Vector<T>> &columns,
                                                                       const std::vector<std::string>& schema,
                                                                       const int& _party_id = 0){
            assert(columns.size() == schema.size());
            using namespace service;
            std::vector<std::shared_ptr<EncodedColumn>> cols;
            for (int i=0; i<columns.size(); i++) {
                auto v = isBShared(schema[i]) ?
                         std::unique_ptr<EncodedVector>(new BSharedVector(runTime.secret_share_b<R>(columns[i], _party_id))) :
                         std::unique_ptr<EncodedVector>(new ASharedVector(runTime.secret_share_a<R>(columns[i], _party_id)));
                cols.push_back(std::shared_ptr<EncodedColumn>(new SharedColumn(std::move(v), schema[i])));
            }
            return cols;
        }
    };

} }


#endif // TVA_ENCODED_TABLE_H
