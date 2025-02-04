// #include <algorithm>
// #include <memory>
// #include <string>
// #include <utility>

// #include "ai_async_inference.hpp"

// #include "glog/logging.h"


// class SampleAsyncInferenceObserver : public infer_server::Observer {
//  public:
//   explicit SampleAsyncInferenceObserver(std::function<void(std::shared_ptr<ai::module::Frame>, bool)> callback)
//                           : callback_(callback) {}
//   void Response(infer_server::Status status, infer_server::PackagePtr data,
//                 infer_server::any user_data) noexcept override {
//     callback_(infer_server::any_cast<std::shared_ptr<ai::module::Frame>>(user_data), status == infer_server::Status::SUCCESS);
//   }

//  private:
//   std::function<void(std::shared_ptr<ai::module::Frame>, bool)> callback_;
// };

// int SampleAsyncInference::Open() {
//   infer_server_.reset(new infer_server::InferServer(device_id_));

//   std::string lower_model_name = model_name_;
//   std::transform(lower_model_name.begin(), lower_model_name.end(), lower_model_name.begin(), ::tolower);

//   infer_server::SessionDesc desc;
//   desc.strategy = infer_server::BatchStrategy::DYNAMIC;
//   desc.engine_num = 6;
//   desc.priority = 0;
//   desc.show_perf = false;
//   desc.batch_timeout = 100;
//   desc.name = "Inference session";

//   // load offline model
//   desc.model = infer_server::InferServer::LoadModel(model_path_);

//   desc.model_input_format = infer_server::NetworkInputFormat::RGB;

//   if (lower_model_name == "yolov3") {
//     preproc_ = std::make_shared<PreprocYolov3>();
//     postproc_ = std::make_shared<PostprocYolov3>();
//   } else if (lower_model_name == "resnet") {
//     preproc_ = std::make_shared<PreprocClassification>();
//     postproc_ = std::make_shared<PostprocClassification>();
//   } else {
//     LOG(ERROR) << "[AI Samples] [SampleAsyncInference] Open: not support this model";
//     return -1;
//   }

//   // set preproc
//   desc.preproc = infer_server::Preprocessor::Create();
//   infer_server::SetPreprocHandler(desc.model->GetKey(), preproc_.get());

//   // set post proc
//   desc.postproc = infer_server::Postprocessor::Create();
//   infer_server::SetPostprocHandler(desc.model->GetKey(), postproc_.get());

//   // set end of frame
//   auto eof_callback_ = [this](std::shared_ptr<ai::module::Frame> frame, bool valid) {
//     if (!valid) {
//       return;
//     }
//     Transmit(frame);
//   };

//   session_ = infer_server_->CreateSession(desc, std::make_shared<SampleAsyncInferenceObserver>(eof_callback_));

//   return 0;
// }

// int SampleAsyncInference::Process(std::shared_ptr<ai::module::Frame> frame) {
//   std::string stream = "stream_0";
//   if (frame->is_eos) {
//     if (infer_server_) infer_server_->WaitTaskDone(session_, stream);
//     Transmit(frame);
//     return 0;
//   }

//   infer_server::PackagePtr request = infer_server::Package::Create(1, stream);
//   infer_server::PreprocInput tmp;

//   tmp.surf = frame->surf;
//   tmp.has_bbox = false;
//   request->data[0]->Set(std::move(tmp));

//   request->data[0]->SetUserData(frame);
//   if (!infer_server_->Request(session_, std::move(request), frame)) {
//     LOG(ERROR) << "[AI Samples] [SampleAsyncInference] Process(): Request infer_server do inference failed";
//     return -1;
//   }
//   return 0;
// }

// int SampleAsyncInference::Close() {
//   if (infer_server_ && session_) {
//     infer_server::RemovePreprocHandler(infer_server_->GetModel(session_)->GetKey());
//     infer_server::RemovePostprocHandler(infer_server_->GetModel(session_)->GetKey());
//     infer_server_->DestroySession(session_);
//   }
//   if (infer_server_) {
//     infer_server_.reset();
//     infer_server_ = nullptr;
//   }
//   return 0;
// }
