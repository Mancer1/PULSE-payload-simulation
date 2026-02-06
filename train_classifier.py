import uproot
import pandas as pd
import numpy as np
import xgboost as xgb
import pickle
import os
from sklearn.model_selection import train_test_split
from sklearn.metrics import accuracy_score, classification_report, confusion_matrix
from sklearn.preprocessing import LabelEncoder

def run_ml_pipeline_root(root_file, tree_name):
    # --- 1. DATA LOADING ---
    print(f"Opening ROOT file: {root_file}")
    
    if not os.path.exists(root_file):
        print(f"Error: File {root_file} not found.")
        return None, None

    with uproot.open(root_file) as file:
        if tree_name not in file:
            print(f"Error: Tree {tree_name} not found in {root_file}")
            return None, None
            
        tree = file[tree_name]
        columns = ["pixel_x", "pixel_y", "charge", "Incident_particle_type"]
        df = tree.arrays(columns, library="pd")
    
    print(f"Loaded {len(df)} pixel hits.")

    # --- 2. PREPROCESSING ---
    features = ["pixel_x", "pixel_y", "charge"]
    X = df[features].to_numpy().astype(float)
    y_raw = df["Incident_particle_type"].to_numpy().astype(str)

    label_encoder = LabelEncoder()
    y = label_encoder.fit_transform(y_raw)
    class_names = label_encoder.classes_

    X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

    # --- 3. TRAINING ---
    print(f"\nTraining XGBoost on {len(X_train)} samples...")
    print(f"Target classes: {class_names}")
    
    model = xgb.XGBClassifier(
        n_estimators=100,
        max_depth=6,
        learning_rate=0.1,
        objective='multi:softprob',
        num_class=len(class_names),
        random_state=42
    )
    
    model.fit(X_train, y_train)

    # --- 4. EVALUATION ---
    # Fix for "inconsistent numbers of samples" [146, 292]
    raw_preds = model.predict(X_test)
    if len(raw_preds.shape) > 1 and raw_preds.shape[1] > 1:
        predictions = np.argmax(raw_preds, axis=1)
    else:
        predictions = raw_preds.astype(int).flatten()
    
    print("\n--- TEST RESULTS ---")
    acc = accuracy_score(y_test, predictions)
    print(f"Overall Accuracy: {acc * 100:.2f}%")
    
    print("\nDetailed Classification Report:")
    print(classification_report(y_test, predictions, target_names=class_names))

    # --- 5. SAVING OUTPUTS ---
    model_name = "particle_classifier.json"
    model.save_model(model_name)
    
    encoder_name = "label_encoder.pkl"
    with open(encoder_name, "wb") as f:
        pickle.dump(label_encoder, f)
        
    print(f"\nSUCCESS: Model saved as '{model_name}' and Encoder as '{encoder_name}'")
    
    return model, label_encoder

if __name__ == "__main__":
    FILE_PATH = "MergedOutput.root"
    TREE_PATH = "pixelcharge_flattened"
    trained_model, encoder = run_ml_pipeline_root(FILE_PATH, TREE_PATH)