import uproot
import pandas as pd
import numpy as np
import xgboost as xgb
import pickle
import os
from tqdm import tqdm
from sklearn.model_selection import train_test_split
from sklearn.metrics import accuracy_score, classification_report, confusion_matrix
from sklearn.preprocessing import LabelEncoder
from sklearn.multioutput import MultiOutputClassifier

def run_ml_pipeline_root(root_file, tree_name):
    # --- DATA LOADING ---
    print(f"Opening ROOT file: {root_file}")
    with uproot.open(root_file) as file:
        tree = file[tree_name]
        columns = ["event_idx", "pixel_x", "pixel_y", "charge", 
                   "Incident_particle_type", "Incident_particle_energy"]
        df = tree.arrays(columns, library="pd")
    
    print(f"Loaded {len(df)} raw pixel hits.")

    print("Grouping hits into events...")
    
    # Define how to squash pixel hits into one event summary
    agg_logic = {
        'charge': ['sum', 'mean', 'count', 'std'], # Total energy, Avg, Cluster Size, Spread
        'pixel_x': ['min', 'max', 'std'],
        'pixel_y': ['min', 'max', 'std'],
        'Incident_particle_type': 'first',
        'Incident_particle_energy': 'first'
    }
    
    event_df = df.groupby('event_idx').agg(agg_logic)
    
    event_df.columns = [f"{col[0]}_{col[1]}" for col in event_df.columns]
    
    # Fill NaN std values (happens if an event has only 1 pixel)
    event_df = event_df.fillna(0)

    # Physics Feature Engineering: Cluster Dimensions
    event_df['cluster_width'] = event_df['pixel_x_max'] - event_df['pixel_x_min']
    event_df['cluster_height'] = event_df['pixel_y_max'] - event_df['pixel_y_min']
    
    print(f"Aggregated into {len(event_df)} unique particle events.")

    # --- PREPROCESSING ---
    features = ['charge_sum', 'charge_mean', 'charge_count', 'charge_std', 
                'cluster_width', 'cluster_height', 'pixel_x_std', 'pixel_y_std']
    
    X = event_df[features].to_numpy().astype(float)
    
    le_type = LabelEncoder()
    le_energy = LabelEncoder()

    y_type = le_type.fit_transform(event_df['Incident_particle_type_first'].astype(str))
    y_energy = le_energy.fit_transform(event_df['Incident_particle_energy_first'].astype(str))

    y = np.column_stack((y_type, y_energy))
    X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

    # --- TRAINING ---
    print(f"\nTraining Multi-Output XGBoost on Event Clusters...")
    base_model = xgb.XGBClassifier(n_estimators=100, max_depth=4, learning_rate=0.1, random_state=42, base_score=0.5)
    model = MultiOutputClassifier(base_model)
    
    with tqdm(total=2, desc="Training") as pbar:
        model.fit(X_train, y_train)
        pbar.update(2)

    # --- EVALUATION ---
    predictions = model.predict(X_test)
    y_test_type, y_test_energy = y_test[:, 0], y_test[:, 1]
    pred_type, pred_energy = predictions[:, 0], predictions[:, 1]

    print("\n--- EVENT-BASED TEST RESULTS ---")
    print(f"Particle Type Accuracy: {accuracy_score(y_test_type, pred_type)*100:.2f}%")
    print(f"Energy Accuracy: {accuracy_score(y_test_energy, pred_energy)*100:.2f}%")
    
    print("\n[Particle Type Report]")
    print(classification_report(y_test_type, pred_type, target_names=le_type.classes_))

    # --- 6. SAVING ---
    with open("event_model.pkl", "wb") as f:
        pickle.dump(model, f)
    with open("event_encoders.pkl", "wb") as f:
        pickle.dump({'type': le_type, 'energy': le_energy}, f)
    print(f"\nSUCCESS: Event-based model saved.")

if __name__ == "__main__":
    FILE_PATH = "MergedOutput.root"
    TREE_PATH = "pixelcharge_flattened"
    run_ml_pipeline_root(FILE_PATH, TREE_PATH)