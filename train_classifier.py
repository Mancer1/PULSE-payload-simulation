import os
import glob
import pandas as pd
import numpy as np
import xgboost as xgb
from sklearn.model_selection import train_test_split
from sklearn.metrics import accuracy_score, classification_report, confusion_matrix
from sklearn.preprocessing import LabelEncoder

def run_ml_pipeline(data_folder):
    print(f"Scanning for HDF5 files in: {data_folder}")
    all_files = glob.glob(os.path.join(data_folder, "*.h5"))
    
    if not all_files:
        print("No HDF5 files found! Check your path.")
        return

    data_list = []
    for file in all_files:
        # Note: 'key' must match the internal name used when saving the H5
        df = pd.read_hdf(file, key='simulation_results')
        data_list.append(df)
    
    full_df = pd.concat(data_list, ignore_index=True)
    print(f"Loaded {len(full_df)} total rows from {len(all_files)} files.")

    # --- 2. PREPROCESSING ---
    # Assume 'target' is your label (e.g., 'proton', 'e-')
    # and all other columns are simulation features
    X = full_df.drop('target', axis=1)
    y_raw = full_df['target']

    # XGBoost requires numbers, so we convert text labels (proton/e-) to 0, 1, 2...
    label_encoder = LabelEncoder()
    y = label_encoder.fit_transform(y_raw)
    class_names = label_encoder.classes_

    X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

    # --- 3. TRAINING ---
    print("\nTraining the XGBoost model...")
    model = xgb.XGBClassifier(
        n_estimators=100,
        max_depth=6,
        learning_rate=0.1,
        objective='multi:softprob',
        num_class=len(class_names),
        random_state=42
    )
    
    model.fit(X_train, y_train)

    # --- 4. TESTING & EVALUATION ---
    print("\n--- TEST RESULTS ---")
    predictions = model.predict(X_test)
    
    # Global Accuracy
    acc = accuracy_score(y_test, predictions)
    errors = (y_test != predictions).sum()
    print(f"Overall Accuracy: {acc * 100:.2f}%")
    print(f"Total Incorrect Predictions: {errors} / {len(y_test)}")

    # Detailed Precision/Recall per particle type
    print("\nDetailed Classification Report:")
    print(classification_report(y_test, predictions, target_names=class_names))

    # Confusion Matrix
    print("Confusion Matrix (Rows=Truth, Columns=Predicted):")
    cm = confusion_matrix(y_test, predictions)
    print(cm)

    # Percentage-based Confusion Matrix for clarity
    cm_perc = cm.astype('float') / cm.sum(axis=1)[:, np.newaxis]
    print("\nConfusion Matrix (Normalized Percentages):")
    print(pd.DataFrame(cm_perc, index=class_names, columns=class_names).round(2))

    return model

if __name__ == "__main__":
    # Change this to your actual folder path
    folder_path = "./sim_data_hdf5" 
    trained_model = run_ml_pipeline(folder_path)