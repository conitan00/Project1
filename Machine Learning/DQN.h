#pragma once

// モデル
class DQN {

private:
    static const int state_size = 2; // 第0層の入力数
    static const int action_size = 8; // 最終層の出力数
    static const int layer_num = 3; // 層の数
public:

    bool save = false;

    // ネットワーク構造

    vector<Convolution2D*> conv;
    vector<Flat*> flat;
    vector<FC*> fc;

    // コンストラクタ
    DQN() {
        conv.push_back(new Convolution2D(2, 16, 9, 9, ActivationID::relu));
        conv.push_back(new Convolution2D(16, 32, 9, 9, ActivationID::relu));
        flat.push_back(new Flat(32, 9, 9));
        //fc.push_back(new FC(32 * STAGE_MAX_Y * STAGE_MAX_X, 32, ActivationID::relu));
        fc.push_back(new FC(32 * 9 * 9, action_size, ActivationID::None));

        if (model) {

            int layer_num = 0;

            for (auto& layer : conv) {
                layer->load_para(layer_num);
                layer_num++;
            }
            for (auto& layer : fc) {
                layer->load_para(layer_num);
                layer_num++;
            }

        }
    }

    // デストラクタ
    ~DQN() {
        for (auto layer : conv) delete layer;
        for (auto layer : flat) delete layer;
        for (auto layer : fc) delete layer;
    }


    // 順伝播(状態を入力し、各行動の値を出力する)
    vector<vector<long double>> forward(const vector<vector<vector<vector<long double>>>>& data, vector<vector<bool>> mask) {
        vector<vector<vector<vector<long double>>>> res = data;
        vector<vector<long double>> output;
        for (auto& layer : conv) {
            res = layer->forward(res);
        }
        for (auto& layer : flat) {
            output = layer->forward(res);
        }
        for (auto& layer : fc) {
            output = layer->forward(output);
        }

        for (int b = 0; b < output.size(); ++b) {
            for (int i = 0; i < action_size; ++i) {
                if (mask[b][i]) { output[b][i] = -100; }
            }
        }

        return output;
    }

    // 逆伝播
    void backward(const vector<vector<long double>>& loss) {
        vector<vector<long double>> d_in = loss;
        vector<vector<vector<vector<long double>>>> d_input;
        for (auto it = fc.rbegin(); it != fc.rend(); ++it) {
            d_in = (*it)->backward(d_in);
        }
        for (auto it = flat.rbegin(); it != flat.rend(); ++it) {
            d_input = (*it)->backward(d_in);
        }
        for (auto it = conv.rbegin(); it != conv.rend(); ++it) {
            d_input = (*it)->backward(d_input);
        }

        if (save) {

            int layer_num = 0;

            for (auto& layer : conv) {
                string name = "para" + to_string(layer_num) + ".csv";
                ofstream file(name, ios::trunc);
                layer->save_para(layer_num);
                layer_num++;
            }
            for (auto& layer : fc) {
                string name = "para" + to_string(layer_num) + ".csv";
                ofstream file(name, ios::trunc);
                layer->save_para(layer_num);
                layer_num++;
            }
            save = false;
            cout << "Saved model." << endl;

        }

    }



};